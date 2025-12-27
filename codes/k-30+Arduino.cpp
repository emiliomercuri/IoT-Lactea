#include <Wire.h>

#define K30_ADDR 0x68

bool readCO2(int &co2) {
  Wire.beginTransmission(K30_ADDR);
  Wire.write(0x22);
  Wire.write(0x00);
  Wire.write(0x08);
  Wire.write(0x2A);
  Wire.endTransmission();

  delay(10);

  int bytes = Wire.requestFrom(K30_ADDR, 4);
  if (bytes != 4) return false;

  byte buffer[4];
  for (int i = 0; i < 4; i++) {
    buffer[i] = Wire.read();
  }

  byte checksum = buffer[0] + buffer[1] + buffer[2];
  if (checksum != buffer[3]) return false;

  co2 = ((int)buffer[1] << 8) | buffer[2];

  if (co2 < 250 || co2 > 10000) return false;

  return true;
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  delay(1000); // deixa o sensor estabilizar
}

void loop() {
  int co2;

  if (readCO2(co2)) {
    Serial.print("{\"co2\":");
    Serial.print(co2);
    Serial.println("}");
  }

  delay(2000);
}
