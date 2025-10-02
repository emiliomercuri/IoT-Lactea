#include <DFRobot_SCD4X.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <RTClib.h>

// RTC
RTC_DS3231 rtc;

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
// HORÁRIO DE CALIBRAÇÃO
// ------------------------
const int horaCalibracao = 14;
const int minutoCalibracao = 30;
bool calibradoHoje = false;  // controle diário

// ------------------------
// FUNÇÕES AUXILIARES
// ------------------------
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("conectado!");
    } else {
      Serial.print("falhou, rc=");
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

void auto_calibration() {
  Serial.println("Iniciando calibração automática...");
  SCD4X.setAutoCalibMode(true);
  delay(60000); // 1 minuto calibrando
  SCD4X.setAutoCalibMode(false);
  Serial.println("Calibração concluída.");
}

// ------------------------
// SETUP
// ------------------------
void setup(void) {
  Serial.begin(115200);
  connectWiFi();
  client.setServer(mqtt_server, mqtt_port);

  // Inicia comunicação I2C e RTC
  Wire.begin();
  rtc.begin();

  // Inicializa o sensor
  while (!SCD4X.begin()) {
    Serial.println("Falha na comunicação com o sensor.");
    delay(3000);
  }

  SCD4X.enablePeriodMeasure(SCD4X_STOP_PERIODIC_MEASURE);
  SCD4X.setTempComp(4.0);
  SCD4X.setSensorAltitude(935);

  SCD4X.enablePeriodMeasure(SCD4X_START_PERIODIC_MEASURE);
  Serial.println("Sensor iniciado com sucesso.");
}

// ------------------------
// LOOP PRINCIPAL
// ------------------------
void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // Verifica horário para calibração
  DateTime now = rtc.now();

  if (now.hour() == horaCalibracao && now.minute() == minutoCalibracao) {
    if (now.second() <= 10 && !calibradoHoje) {   // tolerância de 10s
      auto_calibration();
      calibradoHoje = true;  // evita rodar mais de uma vez no mesmo dia
    }
  }

  // Reseta flag à meia-noite para permitir nova calibração no próximo dia
  if (now.hour() == 0 && now.minute() == 0 && now.second() == 0) {
    calibradoHoje = false;
  }

  // Leitura do sensor
  if (SCD4X.getDataReadyStatus()) {
    DFRobot_SCD4X::sSensorMeasurement_t data;
    SCD4X.readMeasurement(&data);

    Serial.printf("CO2: %d ppm | Temp: %.2f C | Hum: %.2f RH\n", 
                  data.CO2ppm, data.temp, data.humidity);

    // Publicação via MQTT
    char payload[128];
    snprintf(payload, sizeof(payload),
             "{\"co2\":%d, \"temp\":%.2f, \"hum\":%.2f}",
             data.CO2ppm, data.temp, data.humidity);

    client.publish("sensor/ambiente", payload);
  }

  delay(1000); // loop rápido para garantir precisão no RTC
}
