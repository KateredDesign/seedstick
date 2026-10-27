/*
  SeedStick test 05: servo end positions (SG90 on GPIO 18)
  ---------------------------------------------------------
  Finds the two pulse widths at which the metering wheel lines up with the ball path (positions A and B,
  180 degrees apart). Control it from the serial monitor (115200 baud, send each letter with Enter):
    a  select A and move there        b  select B and move there
    +  selected position +10 us        -  selected position -10 us
    w  change A <-> B like a dosing step (pulses for 600 ms, then off)
    o  switch the pulses off           p  print the values
  BUTTON: pressing the BOOT button on the ESP32 board (GPIO 0) does exactly the same as "w".
  Press it only while the program is running, not while the board starts (BOOT held at reset = upload mode).
  The values found go into the firmware as SERVO_US_A and SERVO_US_B (firmware/README.md).

  POWER: the servo is powered from the power module, never from USB or the breadboard.
  With the USB cable plugged in, first pull the yellow 5V wire out of hole 2b, then plug in USB, then switch on.
  Afterwards in this order: main switch off, USB out, yellow wire back into hole 2b.
  If the servo hums at an end position, send "o" at once and take the value back by 20-30 us.
*/
const int PIN_SERVO = 18;
const int PIN_BUTTON = 0;               // BOOT button on the board: pressed = LOW (internal pull-up)
const uint32_t DEBOUNCE_MS = 50;
const uint32_t PWM_HZ = 50;
const uint8_t PWM_BITS = 14;            // 20 ms period = 16384 counts
const int US_MIN = 500, US_MAX = 2500;  // never leave this range
const uint32_t MOVE_MS = 600;           // SG90: about 0.3 s for 180 deg at 4.8 V, plus margin

int usA = 600;   // start values (assumption) - find the real ones with this test
int usB = 2400;
char selected = 'A';
bool atA = true;
uint32_t offAt = 0;
bool btnWasDown = false;
uint32_t btnChangedAt = 0;

void servoPulse(int us) {
  us = constrain(us, US_MIN, US_MAX);
  uint32_t duty = (uint32_t)us * (1UL << PWM_BITS) / 20000UL;
  ledcWrite(PIN_SERVO, duty);
}

void servoOff() { ledcWrite(PIN_SERVO, 0); }

void printValues() {
  Serial.printf("A = %d us, B = %d us, gewaehlt: %c\n", usA, usB, selected);
}

// like command 'w': switch A <-> B, pulses for MOVE_MS, then off
void dosingStep() {
  atA = !atA;
  servoPulse(atA ? usA : usB);
  offAt = millis() + MOVE_MS;
}

// true once per press (falling edge, debounced)
bool buttonPressed() {
  bool down = digitalRead(PIN_BUTTON) == LOW;
  if (down == btnWasDown || millis() - btnChangedAt < DEBOUNCE_MS) return false;
  btnWasDown = down;
  btnChangedAt = millis();
  return down;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  if (!ledcAttach(PIN_SERVO, PWM_HZ, PWM_BITS)) {
    Serial.println("LEDC konnte nicht starten");
  }
  servoOff();  // no pulses until the first command, so the wheel does not jump at power-up
  Serial.println("\n=== Test 05: Servo-Endlagen ===");
  Serial.println("Befehle: a, b, +, -, w, o, p   |   BOOT-Taste = w");
  printValues();
}

void loop() {
  if (offAt && millis() > offAt) {
    servoOff();
    offAt = 0;
  }
  if (buttonPressed()) {
    dosingStep();
    Serial.printf("Taste -> %c\n", atA ? 'A' : 'B');
    printValues();
    return;
  }
  if (!Serial.available()) return;
  char c = Serial.read();
  switch (c) {
    case 'a': selected = 'A'; atA = true; servoPulse(usA); offAt = 0; break;
    case 'b': selected = 'B'; atA = false; servoPulse(usB); offAt = 0; break;
    case '+':
    case '-': {
      int step = (c == '+') ? 10 : -10;
      if (selected == 'A') { usA = constrain(usA + step, US_MIN, US_MAX); servoPulse(usA); }
      else { usB = constrain(usB + step, US_MIN, US_MAX); servoPulse(usB); }
      offAt = 0;
      break;
    }
    case 'w': dosingStep(); break;
    case 'o': servoOff(); offAt = 0; break;
    case 'p': break;
    default: return;  // ignore line endings and unknown characters
  }
  printValues();
}
