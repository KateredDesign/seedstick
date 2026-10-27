/*
  SeedStick test 01: OLED on I2C (no library needed)
  ---------------------------------------------------
  1. Scans the I2C bus and prints every address it finds (the OLED should answer at 0x3C).
  2. Initialises the SSD1306 directly and shows a checkerboard that inverts every 2 s.

  Wiring (electronics/wiring.md): OLED GND -> blue rail, VDD -> red rail (3.3 V),
  SCK -> hole 14j (GPIO 22), SDA -> hole 17j (GPIO 21).
  Expected: serial monitor (115200 baud) shows "0x3C", the display shows a checkerboard.
*/
#include <Wire.h>

const int PIN_SDA = 21;
const int PIN_SCL = 22;
const uint8_t OLED_ADDR = 0x3C;

void oledCommand(uint8_t c) {
  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0x00);  // control byte: command
  Wire.write(c);
  Wire.endTransmission();
}

void oledInit() {
  const uint8_t init[] = {
    0xAE,        // display off
    0xD5, 0x80,  // clock divide
    0xA8, 0x3F,  // multiplex 64
    0xD3, 0x00,  // display offset 0
    0x40,        // start line 0
    0x8D, 0x14,  // charge pump on
    0x20, 0x00,  // horizontal addressing
    0xA1,        // segment remap
    0xC8,        // COM scan direction
    0xDA, 0x12,  // COM pins
    0x81, 0xCF,  // contrast
    0xD9, 0xF1,  // pre-charge
    0xDB, 0x40,  // VCOM detect
    0xA4,        // follow RAM
    0xA6,        // normal (not inverted)
    0x2E,        // scrolling off
    0xAF         // display on
  };
  for (uint8_t c : init) oledCommand(c);
}

void oledCheckerboard() {
  oledCommand(0x21); oledCommand(0); oledCommand(127);  // columns 0..127
  oledCommand(0x22); oledCommand(0); oledCommand(7);    // pages 0..7
  for (int page = 0; page < 8; page++) {
    for (int x = 0; x < 128; x += 16) {
      Wire.beginTransmission(OLED_ADDR);
      Wire.write(0x40);  // control byte: data
      for (int i = 0; i < 16; i++) {
        bool on = (((x + i) / 8) + page) % 2;
        Wire.write(on ? 0xFF : 0x00);
      }
      Wire.endTransmission();
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Test 01: OLED / I2C ===");
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(400000);

  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Gefunden: 0x%02X\n", addr);
      found++;
    }
  }
  if (found == 0) {
    Serial.println("Nichts gefunden: SDA/SCK vertauscht? VDD und GND richtig? (Pinfolge am OLED: GND, VDD, SCK, SDA)");
  }

  oledInit();
  oledCheckerboard();
  Serial.println("Das Display sollte jetzt ein Schachbrett zeigen, das alle 2 s invertiert.");
}

void loop() {
  static bool inverted = false;
  delay(2000);
  inverted = !inverted;
  oledCommand(inverted ? 0xA7 : 0xA6);
}
