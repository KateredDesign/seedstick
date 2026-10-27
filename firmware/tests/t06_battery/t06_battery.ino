/*
  SeedStick test 06: supply voltage of the ESP32 (needs the 100k/100k voltage divider on GPIO 35)
  -------------------------------------------------------------------------------------------------
  Two 100-kilohm resistors divide the 5V node (behind the diode) by two; the middle goes to GPIO 35
  (bridge B4, row 5) with 100 nF to GND. This test prints the measured supply voltage once per second.

  Expected with charged NiMH cells: about 4.6-5.2 V. Below about 4.3 V the ESP32 may restart under servo load
  (start value, check it in your tests). With only USB connected the value shows the USB voltage minus
  the board's own diode, if it has one.
*/
const int PIN_BATT = 35;
const float DIVIDER = 2.0f;  // 100 k / 100 k
const int SAMPLES = 32;

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== Test 06: Versorgungsspannung ===");
}

void loop() {
  uint32_t sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum += analogReadMilliVolts(PIN_BATT);
    delayMicroseconds(200);
  }
  float volts = DIVIDER * (sum / (float)SAMPLES) / 1000.0f;
  Serial.printf("5V-Knoten: %.2f V\n", volts);
  delay(1000);
}
