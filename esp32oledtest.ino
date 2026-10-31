
#include <Wire.h>

void scanI2C(int sdaPin, int sclPin) {
  Serial.println();
  Serial.printf("Scanning SDA=%d, SCL=%d\n", sdaPin, sclPin);

  Wire.end();
  Wire.begin(sdaPin, sclPin);
  Wire.setClock(100000);

  int found = 0;

  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.printf("I2C device found: 0x%02X\n", address);
      found++;
    }
  }

  if (found == 0) {
    Serial.println("No I2C device found.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println("\nESP32 OLED AUTO SCANNER");

  // Standard ESP32 DevKit pins
  scanI2C(21, 22);

  // Alternate pins
  scanI2C(18, 19);

  Serial.println("\nScan finished.");
}

void loop() {
}
