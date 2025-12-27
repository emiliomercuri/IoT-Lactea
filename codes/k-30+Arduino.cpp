#include <Wire.h>

#define K30_ADDR 0x68

void setup() {
  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  Wire.beginTransmission(K30_ADDR);
  Wire.write(0x22);
  Wire.write(0x00);
  Wire.write(0x08);
  Wire.write(0x2A);
  Wire.endTransmission();

  delay(10);

  Wire.requestFrom(K30_ADDR, 4);

  byte buffer[4];
  for (int i = 0; i < 4; i++) {
    buffer[i] = Wire.read();
  }

  byte checksum = buffer[0] + buffer[1] + buffer[2];
  if (checksum != buffer[3]) {
    Serial.println("{\"error\":\"checksum\"}");
    delay(2000);
    return;
  }

  int co2 = (buffer[1] << 8) | buffer[2];

  Serial.print("{\"co2\":");
  Serial.print(co2);
  Serial.println("}");

  delay(2000);
}

