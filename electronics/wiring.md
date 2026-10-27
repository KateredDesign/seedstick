# Electronics and wiring

The electronics sit on a half-size breadboard (400 holes) inside the handle housing. The only soldered sub-assembly
is the power module in its own box on the tube.

## Block diagram

```
4 × AA NiMH ── fuse T2A ── breaker 10 A ── main switch ──┬── VSERVO (red) ── servo SG90 (+)        1000 µF to GND
 4.8 V                                                  │
                                                        └── Schottky 1N5819 ── V5 (yellow) ── ESP32 5V pin   470 µF to GND
                                                                                                 │
                                                                    ESP32 regulator 3.3 V ── OLED, touch, IR, moisture
GND (black): battery minus, both capacitors, servo brown, ESP32 GND, both blue rails
```

- **Rulebook limits:** 4.8 to 5.6 V (below 12 V), peak about 1.2 A (servo 0.7 A, ESP32 0.4 A, modules 0.1 A),
  far below 10 A; fuse **and** circuit breaker as required.
- **Diode:** with USB plugged in, the computer supplies only the ESP32, not the servo or the battery. Together with
  the 470 µF capacitor it also keeps the servo's start-up current away from the ESP32. It costs about 0.4 V.
- **Servo current never runs through the breadboard:** VSERVO goes straight to the servo socket.

## Pin map (ESP32)

| GPIO | Function | Breadboard hole |
|---|---|---|
| 4 | touch pad TTP223, I/O (HIGH = touched) | 24j |
| 16 | green LED (via 150 Ω) | 23j |
| 17 | red LED (via 150 Ω) | 22j |
| 18 | servo signal (orange) | 20j |
| 19 | IR sensor TCRT5000, OUT (LOW = reflection) | 19j |
| 21 / 22 | OLED SDA / SCK (I²C, address 0x3C) | 17j / 14j |
| 34 | moisture sensor AOUT (ADC1) | 7b (bridge B3) |
| 35 | supply voltage, middle of a 100 k / 100 k divider (ADC1) | 5d (bridge B4) |

## Breadboard

The ESP32 covers one of its pin rows, so four flat wire bridges run **under** the board (lay them before plugging
the ESP32 in):

| Bridge | From → to | Purpose |
|---|---|---|
| B1 | 30e → centre channel → 2e | 5V pin to row 2 (V5 input) |
| B2 | 12b → 9b | 3V3 pin to row 9 |
| B3 | 16c → 7c | GPIO 34 to row 7 (moisture) |
| B4 | 17d → 5d | GPIO 35 to row 5 (supply sense) |

ESP32 in columns a and i, rows 12–30, USB-C to the right. Then:

| Jumper / part | From → to |
|---|---|
| J1 | 9a → lower red rail (3.3 V) |
| J2 | lower red → upper red rail |
| J3 | lower blue → upper blue rail |
| J4, J5 | 12j and 18j (GND pins) → upper blue rail |
| C1 100 nF | 7a → lower blue rail (moisture signal filter) |
| R1 100 kΩ | 2c → 5c |
| R2 100 kΩ | 5a → lower blue rail |
| C2 100 nF | 5b → lower blue rail |

Cables:

| Cable | Hole |
|---|---|
| power module yellow (V5) | 2b |
| power module black, moisture GND, IR GND | lower blue rail |
| moisture VCC, IR VCC | lower red rail |
| moisture AOUT | 7b |
| IR OUT | 19j |
| servo signal (orange) | 20j |
| touch I/O | 24j |
| touch VCC | upper red rail |
| touch GND, LED cathodes | upper blue rail |
| LED green anode / LED red anode | 23j / 22j |
| OLED SDA / SCK | 17j / 14j |
| OLED VDD / GND | upper red / blue rail |

Servo plug on the 3-pin socket of the harness: red in the middle, **brown on the pin marked GND**, orange on S.

## Power module (soldered)

1. Battery red → fuse holder (SZ2030) → inline holder with the 10 A breaker (SZ2040 + SF2333) → main switch (KCD11)
   → node **VSERVO**.
2. VSERVO: red output wire, plus of the 1000 µF capacitor, anode of the 1N5819 (end without the ring).
3. **V5** at the diode's cathode (ring): yellow output wire, plus of the 470 µF capacitor.
4. GND: battery black, minus of both capacitors, black output wire.
5. Harness end: 3-pin servo socket (red middle, black GND), orange 8 cm to 20j, black branch 8 cm to the blue rail,
   yellow to 2b.

**Check with full cells, switch on:** red to black 5.0–5.6 V; yellow 0.2–0.4 V lower. Yellow at 0 V means the diode
is the wrong way round or a joint is cold.

## Safety rules

1. **Main switch off before plugging in USB.** Otherwise the battery can feed back into the computer's USB port.
2. To run the servo while USB is plugged in (servo set-up): pull the yellow plug out of 2b, plug in USB, switch on.
   Afterwards: switch off, USB out, yellow plug back into 2b.
3. Better still, use a powered USB hub between computer and ESP32, or the web console over Wi-Fi
   ([firmware/README.md](../firmware/README.md)).
