# Bill of materials

Prices in AUD incl. GST (website list prices or our own entries). Parts from the Build-a-thon event shop were paid with tickets; small parts came from the MCIC makerspace. The printed parts are listed in [print-settings.md](print-settings.md). Spreadsheet version: [bom.xlsx](bom.xlsx).

| No. | Category | Part | Specification | Used for | Qty | Unit price | Source |
|---|---|---|---|---|---|---|---|
| P01 | Tubes | Electrical conduit, grey, rigid, 25 mm Medium Duty | DETA 4 m x 25 mm MD rigid PVC conduit, AS/NZS 2053 (Bunnings p0829448) | Lower tube 554 mm, upper tube 300 mm | 1 length (4 m) | 6.57 | Bunnings |
| P02 | Tubes | Plain coupling for 25 mm conduit, grey | DETA 25 mm grey PVC conduit plain coupling (Bunnings p0963383) | Couplings A and B | 4 pcs | 0.35 | Bunnings |
| P03 | Tubes | Polycarbonate tube 22 ID x 25 OD x 305 mm | uxcell, Amazon AU B08LYZ6KQ5 | Sight tube 140 mm, magazine tube 155 mm | 1 pcs | 25.19 | Amazon AU |
| P04 | Power | Panel fuse holder M205 (5 x 20 mm), screw cap | Jaycar SZ2030 | Fine fuse in the power module front | 1 pcs | 2.35 | Jaycar |
| P05 | Power | Glass fuse 5 x 20 mm, T2A (slow blow) | M205 T2A | Fine fuse (one spare) | 2 pcs | 1.40 | Jaycar |
| P06 | Power | Inline blade fuse holder (ATO) with leads | Jaycar SZ2040 | Holds the circuit breaker in the power module | 1 pcs | 4.10 | Jaycar |
| P07 | Power | Circuit breaker 10 A, manual reset, standard blade | Jaycar SF2333 | Required breaker (rulebook 6.1) | 1 pcs | 6.59 | Jaycar |
| P08 | Electronics | Electrolytic capacitor 1000 uF, >= 10 V, radial | max. 10 x 20 mm | Buffer at the servo (VSERVO) | 2 pcs | 0.85 | Jaycar or Core Electronics |
| P09 | Electronics | Electrolytic capacitor 470 uF, >= 10 V, radial |  | Buffer at the ESP32 5V pin (V5) | 2 pcs | 0.50 | Jaycar or Core Electronics |
| P10 | Electronics | Schottky diode 1N5819, 1 A 40 V | Jaycar ZR1020 | Between VSERVO and the ESP32 5V pin | 2 pcs | 0.80 | Jaycar |
| P11 | Electronics | ESP32 development board, 38 pins, USB-C | ESP32-WROOM (CP210x) | Controller | 1 pcs | – | Event shop (ticket) |
| P12 | Electronics | Breadboard 400 holes | half size | Wiring in the handle housing | 1 pcs | – | Event shop (ticket) |
| P13 | Electronics | Dupont jumper wires (F-M, M-M) |  | Module connections, plug ends of the cables | 1 set | – | Event shop (ticket) |
| P14 | Electronics | USB-C cable |  | Programming | 1 pcs | – | Event shop (ticket) |
| P15 | Electronics | OLED display 0.96 inch, I2C (SSD1306) | GM009605 v4.3, 128 x 64 | Display in the front panel | 1 pcs | – | Event shop (ticket) |
| P16 | Electronics | IR reflective sensor module TCRT5000 | 3 pins (GND, OUT, VCC) | Counts the balls (irClip) | 1 pcs | – | Event shop (ticket) |
| P17 | Electronics | Micro servo SG90 |  | Metering wheel | 1 pcs | – | Event shop (ticket) |
| P18 | Electronics | Soil moisture sensor, capacitive v2.0 | HW-390 type | Moisture in the soil (foot) | 1 pcs | – | Event shop (ticket) |
| P19 | Power | Battery holder 4 x AA with leads |  | Battery pack | 1 pcs | – | Event shop (ticket) |
| P20 | Electronics | LED 5 mm, red and green |  | Status LEDs in the front panel | 2 pcs | – | Event shop (ticket) |
| P21 | Power | Rocker switch (KCD11) |  | Main switch | 1 pcs | – | Event shop (ticket) |
| P22 | Electronics | Capacitive touch module TTP223 |  | Touch button on the handle | 1 pcs | – | Event shop (ticket) |
| P23 | Hardware | Heat-set insert M2 | brass, 3.5 mm OD | OLED bracket 2, servo cover 2 | 4 pcs | – | MCIC Makerspace |
| P24 | Hardware | Heat-set insert M3 | brass, 5 mm OD | Sight cover 4, battery lid 4 | 8 pcs | – | MCIC Makerspace |
| P25 | Hardware | Screw M2 x 6 |  | OLED bracket 2, servo cover 2 | 4 pcs | – | MCIC Makerspace |
| P26 | Hardware | Screw M2 x 12 |  | Jaw pivots on the foot | 2 pcs | – | MCIC Makerspace |
| P27 | Hardware | Screw M2.5 x 6 |  | IR module in the irClip | 2 pcs | – | MCIC Makerspace |
| P28 | Hardware | Screw M2.5 x 8 |  | Front panel | 4 pcs | – | MCIC Makerspace |
| P29 | Hardware | Screw M3 x 8 |  | Sight cover 4, battery lid 4 | 8 pcs | – | MCIC Makerspace |
| P30 | Hardware | Screw M4 x 20 |  | Grip clamps 3, power module 1, coupling clamps 8, foot 1, sensor holder 2 | 15 pcs | – | MCIC Makerspace |
| P31 | Hardware | Nut M4 (7 mm AF) |  | With the M4 x 20 screws | 15 pcs | – | MCIC Makerspace |
| P32 | Hardware | Nut M2.5 (5 mm AF) |  | Front panel | 4 pcs | – | MCIC Makerspace |
| P33 | Electronics | Resistor 150 ohm, 1/4 W |  | LED series resistors | 2 pcs | – | MCIC Makerspace |
| P34 | Electronics | Resistor 100 kohm, 1/4 W |  | Battery voltage divider (GPIO 35) | 2 pcs | – | MCIC Makerspace |
| P35 | Consumables | Hook-up wire 24 / 26 / 28 AWG (red, black, yellow, orange) |  | Harness, moisture, IR, touch, LED cables | 1 set | – | MCIC Makerspace |
| P36 | Consumables | Solid wire 0.6 mm (22 AWG) |  | 4 bridges under the ESP32 | 1 set | – | MCIC Makerspace |
| P37 | Consumables | Pin header 2.54 mm, 3 pins |  | Servo socket | 1 pcs | – | MCIC Makerspace |
| P38 | Consumables | Solder, heat shrink 1.5-6 mm, hot glue |  | Every solder joint, securing plugs | 1 set | – | MCIC Makerspace |
| P39 | Consumables | 2-part epoxy (5 or 30 min) |  | Horn glued into the metering wheel | 1 set | – | MCIC Makerspace |
| P40 | Laser part | Acrylic (PMMA) 3 mm, clear, offcut approx. 70 x 70 mm | hardware/laser/sight-cover.dxf | Sight cover of the metering module (laser cut) | 1 pcs | – | MCIC Makerspace |
| P41 | Consumables | Cyanoacrylate glue (super glue) and 1.75 mm filament offcut |  | Gluing the 12 dummy balls (2 pins of 4.6 mm each) | 1 set | – | MCIC Makerspace |

**Total spent on purchased parts: AUD 53.30** (plus filament).
