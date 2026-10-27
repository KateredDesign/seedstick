# Firmware

Arduino sketches for the ESP32 dev board.

| Folder | What it is |
|---|---|
| `seedstick/` | **Main firmware 1.2**: moisture bands, dosing with infrared confirmation, logging, Wi-Fi page, web console |
| `tests/t01`–`t06` | one small test sketch per module: OLED, LEDs and touch, IR sensor, moisture, servo, supply voltage |

The display and serial texts of the firmware are in German.

## Tools

- Arduino IDE 2
- Board package **esp32 by Espressif, 3.3.12** (Boards Manager URL
  `https://espressif.github.io/arduino-esp32/package_esp32_index.json`)
- Library **U8g2 by oliver, 2.36.19** (Library Manager)
- Board **ESP32 Dev Module**, partition scheme **Default 4MB with spiffs** (the log file lives in LittleFS)
- USB driver CP210x or CH9102, depending on the board

## Upload

1. **Main switch off**, then plug in USB (see the safety rules in [electronics/wiring.md](../electronics/wiring.md)).
2. Open `seedstick/seedstick.ino`; `config.h` opens as a second tab.
3. Optional: copy `secrets.h.example` to `secrets.h` and set your own Wi-Fi password (at least 8 characters). This
   protects the access point and enables the web console. `secrets.h` is ignored by git.
4. Verify (✓) and upload (→). If the upload hangs at "Connecting…", hold the BOOT button on the board.
5. Serial monitor: 115200 baud, line ending "New Line". After every reset the firmware prints a start report.

## Operation (main firmware)

One touch pad does everything:

| Gesture | Action |
|---|---|
| tap (up to 0.8 s) | set one seed ball |
| hold 2 s, release | Wi-Fi access point on / off |
| hold 5 s, release | calibrate the moisture sensor: air, tap; water up to the line, tap |

The OLED shows the moisture band (`TROCKEN` / `GEEIGNET` / `NASS` = dry / suitable / wet), the percentage, the ball
count and hints such as "lift, then tap". The green LED means "suitable, plant here"; the red LED flashes twice when
the infrared sensor has counted the ball, and blinks slowly when the battery is low (below 4.3 V).

Bands: below 30 % dry, above 70 % wet, in between suitable (`BAND_DRY_PCT`, `BAND_WET_PCT` in `config.h`).

## Serial commands

| Command | Action |
|---|---|
| `a` / `b` | move the servo to end position A / B and select it |
| `+` / `-` | change the selected end position by 10 µs |
| `w` | switch A ↔ B like a dosing step (without counting) |
| `o` | servo pulses off |
| `save` | store end positions A and B |
| `status` | print all values |
| `cal` | calibrate the moisture sensor (then tap the touch pad) |
| `log` / `clear` | print / clear the log and the counter |
| `test` | guided self-test of every part |
| `rotate` | turn the display by 180° (stored) |
| `help` | list of commands |

**Setting the metering wheel:** send `a`, adjust with `+`/`-` until one pocket of the wheel points straight up; the
same with `b`; check with a few `w`; then `save`.

## Wi-Fi page and web console

Hold the touch pad for 2 s: the stick opens the access point **"SeedStick"**.

- **http://192.168.4.1**: moisture band, counter and supply voltage (live), the log as a table, "download log.csv"
  and "clear log". The page sets the stick's clock from the phone.
- **http://192.168.4.1/console** (firmware 1.2, only with a password in `secrets.h`): shows everything the serial
  monitor would show and takes the same commands, with buttons for the common ones. No USB cable is needed to set
  up the stick.

Log columns: `nr, zeit_s, uhrzeit, feuchte_pct, feuchte_mv, band, ergebnis, versorgung_v` (number, seconds since
start, time, moisture %, moisture mV, band, result, supply voltage).

## Test sketches

| Sketch | Checks |
|---|---|
| `t01_oled_i2c` | I²C scan and a checkerboard on the OLED (no library needed) |
| `t02_leds_touch` | both LEDs and the touch pad |
| `t03_ir_sensor` | infrared sensor output (LOW = reflection) |
| `t04_moisture` | moisture reading in mV (air vs. water, at least 800 mV apart) |
| `t05_servo` | servo end positions (`a`, `b`, `+`, `-`, `w`, `o`, `p`; the BOOT button acts like `w`) |
| `t06_battery` | supply voltage through the divider on GPIO 35 |
