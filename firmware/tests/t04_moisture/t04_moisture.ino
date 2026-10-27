/*
  SeedStick test 04: capacitive soil moisture sensor v2.0 (analogue)
  -------------------------------------------------------------------
  Reads GPIO 34 (ADC1) as millivolts, averages 32 samples and prints the value twice per second together
  with the lowest and highest value since the start.

  Wiring: sensor VCC -> red rail (3.3 V), GND -> blue rail, AOUT -> hole 7b (row 7, bridge B3 to GPIO 34);
  100 nF (C1) from hole 7a to the blue rail.
  Do this: hold the sensor in the air (dry), then put it into a glass of water up to the line on the board.
  The value must change clearly (typically by more than 800 mV); dry gives the HIGHER value.
  If it hardly changes, the sensor board is probably faulty ; swap it.
  Write both values down: they are the start values for the calibration in the firmware.
*/
const int PIN_MOIST = 34;
const int SAMPLES = 32;

uint32_t readMilliVolts() {
  uint32_t sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum += analogReadMilliVolts(PIN_MOIST);
    delayMicroseconds(200);
  }
  return sum / SAMPLES;
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== Test 04: Feuchtesensor ===");
  Serial.println("Erst in der Luft halten, dann ins Wasser (nur bis zur Linie auf der Platine).");
}

void loop() {
  static uint32_t lo = UINT32_MAX, hi = 0;
  uint32_t mv = readMilliVolts();
  lo = min(lo, mv);
  hi = max(hi, mv);
  Serial.printf("Feuchte: %4lu mV   (kleinster %4lu, groesster %4lu)\n",
                (unsigned long)mv, (unsigned long)lo, (unsigned long)hi);
  delay(500);
}
