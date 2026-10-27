/*
  SeedStick test 03: IR sensor (TCRT5000 module with 3 pins: GND, OUT, VCC)
  ------------------------------------------------------------------------
  The module has no analogue output; OUT comes from its comparator (LM393). On these modules OUT goes LOW
  while the sensor sees a reflection, and the small LED "D1" on the module lights.

  This test counts every falling edge with an interrupt (a falling ball gives a pulse of only a few
  milliseconds) and prints the state four times per second. The red LED shows the current state.

  Wiring: IR VCC -> red rail (3.3 V), GND -> blue rail, OUT -> hole 19j (GPIO 19).
  Set the potentiometer so that D1 is OFF with the empty tube and ON with a ball held in front of the window.
  Then drop balls through the tube: the counter must go up by exactly one per ball.
*/
const int PIN_IR = 19;
const int PIN_LED_RED = 17;
const uint32_t DEBOUNCE_US = 20000;  // ignore further edges for 20 ms after a pulse

volatile uint32_t pulses = 0;
volatile uint32_t lastPulseUs = 0;

void IRAM_ATTR onIrFalling() {
  uint32_t now = micros();
  if (now - lastPulseUs > DEBOUNCE_US) {
    pulses = pulses + 1;  // not "pulses++": deprecated on volatile in C++20
    lastPulseUs = now;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_IR, INPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_IR), onIrFalling, FALLING);
  Serial.println("\n=== Test 03: IR-Sensor ===");
  Serial.println("OUT = 0: Reflexion erkannt (LED D1 am Modul an); OUT = 1: frei.");
}

void loop() {
  static uint32_t lastPrint = 0;
  static uint32_t lastCount = 0;
  int out = digitalRead(PIN_IR);
  digitalWrite(PIN_LED_RED, out == LOW);
  if (millis() - lastPrint >= 250) {
    lastPrint = millis();
    uint32_t count = pulses;
    Serial.printf("OUT = %d   Kugeln/Pulse: %lu%s\n", out, (unsigned long)count,
                  count != lastCount ? "   <- neu" : "");
    lastCount = count;
  }
}
