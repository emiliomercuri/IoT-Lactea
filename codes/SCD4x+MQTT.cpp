#include <DFRobot_SCD4X.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// ------------------------
// CONFIGURAÇÕES DE WIFI E MQTT
// ------------------------
const char* ssid = "lactea-rasp4-thiago";
const char* password = "iotempire";
const char* mqtt_server = "200.XX.XXX.XX"; // Ex: "192.168.1.100"
const int mqtt_port = 1883;
const char* mqtt_user = "pi"; // Se necessário
const char* mqtt_password = "iot******"; // Se necessário

WiFiClient espClient;
PubSubClient client(espClient);

// ------------------------
// SENSOR SCD4X
// ------------------------
DFRobot_SCD4X SCD4X(&Wire, SCD4X_I2C_ADDR);

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

// ------------------------
// SETUP
// ------------------------
void setup(void) {
  Serial.begin(115200);
  connectWiFi();
  client.setServer(mqtt_server, mqtt_port);

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

  if (SCD4X.getDataReadyStatus()) {
    DFRobot_SCD4X::sSensorMeasurement_t data;
    SCD4X.readMeasurement(&data);

    Serial.printf("CO2: %d ppm | Temp: %.2f C | Hum: %.2f RH\n", data.CO2ppm, data.temp, data.humidity);

    // Publicação via MQTT
    char payload[128];
    snprintf(payload, sizeof(payload),
             "{\"co2\":%d, \"temp\":%.2f, \"hum\":%.2f}",
             data.CO2ppm, data.temp, data.humidity);

    client.publish("sensor/ambiente", payload);
  }

  delay(5000);  // Espera entre medições
}

