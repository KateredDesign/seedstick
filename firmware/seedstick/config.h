// SeedStick firmware: pins and start values (firmware/README.md, wiring in electronics/wiring.md).
// Values marked "start value" are set during calibration; most of them are stored in flash at run time.
#pragma once
#include <Arduino.h>
#include <WiFi.h>

// ---------------------------------------------------------------------------------------------------- pins
constexpr int PIN_TOUCH = 4;       // TTP223 I/O, HIGH = touched (hole 24j)
constexpr int PIN_LED_GREEN = 16;  // through 150 ohm in the LED cable (hole 23j)
constexpr int PIN_LED_RED = 17;    // through 150 ohm in the LED cable (hole 22j)
constexpr int PIN_SERVO = 18;      // SG90 signal (hole 20j)
constexpr int PIN_IR = 19;         // TCRT5000 module OUT, LOW = reflection (hole 19j)
constexpr int PIN_SDA = 21;        // OLED SDA (hole 17j)
constexpr int PIN_SCL = 22;        // OLED SCK (hole 14j)
constexpr int PIN_MOIST = 34;      // moisture sensor AOUT, ADC1 (bridge B3, row 7)
constexpr int PIN_SUPPLY = 35;     // middle of the 100k/100k divider on the 5V node, ADC1 (bridge B4, row 5)

// ---------------------------------------------------------------------------------------------------- display
constexpr uint8_t OLED_I2C_ADDR = 0x3C;  // GM009605 module (test t01 finds it there)
constexpr bool OLED_ROTATE_180 = false;  // start value; the serial command "rotate" turns the text and stores it

// ---------------------------------------------------------------------------------------------------- servo
constexpr int SERVO_US_A_DEFAULT = 600;   // start value; set with the serial commands a, b, +, -, save
constexpr int SERVO_US_B_DEFAULT = 2400;  // start value
constexpr int SERVO_US_MIN = 500;         // never leave this range
constexpr int SERVO_US_MAX = 2500;
constexpr int SERVO_STEP_US = 10;         // step of the serial commands + and -
constexpr uint32_t SERVO_MOVE_MS = 600;   // pulses this long, then off (SG90: about 0.3 s for 180 deg)

// ---------------------------------------------------------------------------------------------------- IR sensor
constexpr int IR_ACTIVE_LEVEL = LOW;      // level while the sensor sees a reflection
constexpr uint32_t IR_WINDOW_MS = 1500;   // time for the confirmation after the servo start
constexpr uint32_t IR_IGNORE_MS = 100;    // no edge counts this early (the fall to the sensor takes about 0.3 s)
constexpr uint32_t IR_DEBOUNCE_MS = 20;   // further edges ignored after one pulse
constexpr uint32_t IR_BLOCKED_MS = 1000;  // active this long while idle = sensor blocked

// ---------------------------------------------------------------------------------------------------- touch
constexpr uint32_t TOUCH_SETTLE_MS = 600;  // TTP223 calibrates after power-up: do not evaluate before
constexpr uint32_t TOUCH_DEBOUNCE_MS = 20;
constexpr uint32_t TAP_MAX_MS = 800;       // shorter = set one ball
constexpr uint32_t HOLD_WIFI_MS = 2000;    // 2 s = Wi-Fi on/off
constexpr uint32_t HOLD_CAL_MS = 5000;     // 5 s = moisture calibration
constexpr uint32_t LOCKOUT_MS = 1500;      // no new tap after a dose

// ---------------------------------------------------------------------------------------------------- moisture
constexpr uint32_t MOIST_INTERVAL_MS = 250;
constexpr int MOIST_SAMPLES = 16;           // ADC samples per reading
constexpr int MOIST_AVG = 8;                // moving average over this many readings
constexpr uint32_t MOIST_SERVO_QUIET_MS = 200;  // no reading while the servo runs and this long after
constexpr int MOIST_DRY_MV_DEFAULT = 2500;  // start value until the first calibration (sensor in air)
constexpr int MOIST_WET_MV_DEFAULT = 1200;  // start value (sensor in water)
constexpr int MOIST_MIN_SPAN_MV = 300;      // dry must be at least this much above wet, else calibration fails
constexpr int BAND_DRY_PCT = 30;            // below: TROCKEN
constexpr int BAND_WET_PCT = 70;            // above: NASS; in between: GEEIGNET

// ---------------------------------------------------------------------------------------------------- supply voltage
constexpr float SUPPLY_DIVIDER = 2.0f;      // 100 k / 100 k
constexpr uint32_t SUPPLY_INTERVAL_MS = 1000;
constexpr float SUPPLY_WARN_V = 4.3f;       // start value: warning "Akku schwach"
constexpr float SUPPLY_LOW_V = 4.1f;        // start value: "Akku leer - laden"

// ---------------------------------------------------------------------------------------------------- self-test ("test")
constexpr uint32_t TEST_WAIT_MS = 10000;      // longest wait per step (touch, IR); Enter skips the step
constexpr int MOIST_PLAUSIBLE_MIN_MV = 300;   // below: no signal from the moisture sensor
constexpr int MOIST_PLAUSIBLE_MAX_MV = 3000;  // above: reading at the stop of the ADC (wiring)

// ---------------------------------------------------------------------------------------------------- Wi-Fi and log
constexpr const char* WIFI_SSID = "SeedStick";
constexpr wifi_power_t WIFI_TX_POWER = WIFI_POWER_8_5dBm;  // lower current peaks on the battery
constexpr const char* LOG_PATH = "/log.csv";
// A password is optional: put   #define WIFI_PASSWORD "at-least-8-characters"   into secrets.h next to this
// file. secrets.h is listed in .gitignore and never goes into the repository. Without it the network is open.
