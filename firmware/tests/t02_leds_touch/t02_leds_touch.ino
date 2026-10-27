/*
  SeedStick test 02: status LEDs and touch button
  ------------------------------------------------
  - At start both LEDs light for 1 s (checks the LED cables and the 150-ohm resistors).
  - Afterwards the green LED follows the touch pad and the red LED blinks once per second.
  - Every change of the touch pad is printed on the serial monitor (115200 baud).

  Wiring: LED green -> hole 23j (GPIO 16), LED red -> hole 22j (GPIO 17), each through its 150-ohm resistor
  in the LED cable; LED cathodes -> blue rail. Touch I/O -> hole 24j (GPIO 4), VCC -> red rail, GND -> blue rail.
  Do not touch the pad while switching on: the TTP223 calibrates itself in the first half second.
*/
const int PIN_LED_GREEN = 16;
const int PIN_LED_RED = 17;
const int PIN_TOUCH = 4;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_TOUCH, INPUT);
  digitalWrite(PIN_LED_GREEN, HIGH);
  digitalWrite(PIN_LED_RED, HIGH);
  delay(1000);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_RED, LOW);
  Serial.println("\n=== Test 02: LEDs und Touch ===");
  Serial.println("Beide LEDs haben 1 s geleuchtet? Jetzt das Touch-Pad antippen: gruen leuchtet, solange ihr beruehrt.");
}

void loop() {
  static int lastTouch = -1;
  static uint32_t lastBlink = 0;
  static bool red = false;

  int touch = digitalRead(PIN_TOUCH);
  digitalWrite(PIN_LED_GREEN, touch);
  if (touch != lastTouch) {
    Serial.printf("%8lu ms  Touch: %s\n", (unsigned long)millis(), touch ? "beruehrt" : "frei");
    lastTouch = touch;
  }
  if (millis() - lastBlink >= 500) {
    lastBlink = millis();
    red = !red;
    digitalWrite(PIN_LED_RED, red);
  }
  delay(5);
}
