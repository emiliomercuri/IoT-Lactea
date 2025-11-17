#include <DFRobot_SCD4X.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <time.h>

// ------------------------
// CONFIG WIFI E MQTT
// ------------------------
const char* ssid = "*********";
const char* password = "******";
const char* mqtt_server = "********";
const int mqtt_port = 1883;
const char* mqtt_user = "********";
const char* mqtt_password = "********";

WiFiClient espClient;
PubSubClient client(espClient);

// ------------------------
// SENSOR SCD4X
// ------------------------
DFRobot_SCD4X SCD4X(&Wire, SCD4X_I2C_ADDR);

// ------------------------
// VÁRIOS HORÁRIOS + LIMITE DE 30 MINUTOS ENTRE CALIBRAÇÕES
// ------------------------
struct Horario {
  int hora;
  int minuto;
  bool calibrado;
  unsigned long ultimaCalibracao;  // timestamp em segundos
};

// Horários programados para calibração automática:
Horario horarios[] = {
  { 9,  0, false, 0 },
  { 12, 0, false, 0 },
  { 15, 30, false, 0 },
  { 20, 0, false, 0 }
};

const int totalHorarios = sizeof(horarios) / sizeof(horarios[0]);
const unsigned long tempoReset = 30 * 60;  // 30 minutos em segundos

bool calibrando = false;

// ------------------------
// FUNÇÕES AUXILIARES
// ------------------------
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println(" conectado!");
      client.publish("sensor/status", "MQTT conectado", true);
    } else {
      Serial.print(" falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5s...");
      delay(5000);
    }
  }
}

void connectWiFi() {
  Serial.print("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(" conectado!");
}

void initTime() {
  configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // Fuso horário -3
  Serial.print("Aguardando sincronização NTP");
  time_t now = time(nullptr);
  while (now < 100000) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println(" sincronizado!");
}

// ------------------------
// CALIBRAÇÃO
// ------------------------
void auto_calibration() {
  calibrando = true;

  client.publish("sensor/status", "Iniciando calibração automática...", true);
  Serial.println("Iniciando calibração automática...");

  SCD4X.setAutoCalibMode(true);

  unsigned long start = millis();
  while (millis() - start < 60000) {  // 60 segundos
    client.loop();
    delay(100);
  }

  SCD4X.setAutoCalibMode(false);
  calibrando = false;

  client.publish("sensor/status", "Calibração concluída com sucesso!", true);
  Serial.println("Calibração concluída.");
}

void resetarCalibracoesDia() {
  for (int i = 0; i < totalHorarios; i++) {
    horarios[i].calibrado = false;
    horarios[i].ultimaCalibracao = 0;
  }
  Serial.println("Calibrações resetadas para o novo dia.");
}

// ------------------------
// SETUP
// ------------------------
void setup() {
  Serial.begin(115200);
  connectWiFi();
  initTime();
  client.setServer(mqtt_server, mqtt_port);

  while (!SCD4X.begin()) {
    Serial.println("Falha na comunicação com o sensor.");
    delay(3000);
  }

  SCD4X.enablePeriodMeasure(SCD4X_STOP_PERIODIC_MEASURE);
  SCD4X.setTempComp(4.0);
  SCD4X.setSensorAltitude(935);
  SCD4X.enablePeriodMeasure(SCD4X_START_PERIODIC_MEASURE);

  Serial.println("Sensor iniciado com sucesso.");
  client.publish("sensor/status", "Sensor iniciado com sucesso", true);
}

// ------------------------
// LOOP PRINCIPAL
// ------------------------
void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!client.connected()) reconnectMQTT();
  client.loop();

  // Obtém hora atual
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  // Verifica todos os horários
  for (int i = 0; i < totalHorarios; i++) {

    // Se já calibrado, verifica se já passaram 30 minutos
    if (horarios[i].calibrado && horarios[i].ultimaCalibracao > 0) {
      if ((now - horarios[i].ultimaCalibracao) >= tempoReset) {
        horarios[i].calibrado = false;
        horarios[i].ultimaCalibracao = 0;
        Serial.printf("Horário %02d:%02d liberado novamente após 30 minutos.\n",
                      horarios[i].hora, horarios[i].minuto);
      }
    }

    // Iniciar calibração se horário bate
    if (!horarios[i].calibrado &&
        timeinfo->tm_hour == horarios[i].hora &&
        timeinfo->tm_min == horarios[i].minuto) {

      Serial.printf("Iniciando calibração programada das %02d:%02d\n",
                    horarios[i].hora, horarios[i].minuto);

      auto_calibration();

      horarios[i].calibrado = true;
      horarios[i].ultimaCalibracao = now;  // registra timestamp
    }
  }

  // Reset diário à meia-noite
  if (timeinfo->tm_hour == 0 &&
      timeinfo->tm_min == 0) {
    resetarCalibracoesDia();
  }

  // Leitura do sensor
  if (SCD4X.getDataReadyStatus()) {
    DFRobot_SCD4X::sSensorMeasurement_t data;
    SCD4X.readMeasurement(&data);

    char datetimeStr[25];
    snprintf(datetimeStr, sizeof(datetimeStr),
             "%04d-%02d-%02d %02d:%02d:%02d",
             timeinfo->tm_year + 1900,
             timeinfo->tm_mon + 1,
             timeinfo->tm_mday,
             timeinfo->tm_hour,
             timeinfo->tm_min,
             timeinfo->tm_sec);

    Serial.printf("[%s] CO2: %d ppm | Temp: %.2f C | Hum: %.2f RH\n",
                  datetimeStr, data.CO2ppm, data.temp, data.humidity);

    char payload[256];
    snprintf(payload, sizeof(payload),
             "{\"datetime\":\"%s\",\"co2\":%d,\"temp\":%.2f,\"hum\":%.2f}",
             datetimeStr, data.CO2ppm, data.temp, data.humidity);

    client.publish("sensor/ambiente", payload, true);
  }

  delay(5000);
}
