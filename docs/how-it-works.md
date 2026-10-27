# How SeedStick works

<p align="center"><img src="images/seedstick-render.jpg" alt="SeedStick prototype" width="380"></p>

The planting cycle as an interactive figure: [index.html](index.html) (dark: [dark.html](dark.html)); use ← → to step
through the cycle, space to pause.

## Layout of the stick

From the tip upwards:

| Section | Parts |
|---|---|
| **Foot** | conical tip of two jaws held closed by an O-ring (22 × 1), depth collar Ø 80 mm at 30 mm, socket for the lower tube |
| **Moisture sensor** | capacitive sensor board in a clamp-on holder; its blade runs through a slot in the collar into the soil |
| **Lower tube** | 25 mm PVC conduit, 554 mm |
| **Sight tube** | clear polycarbonate tube, 140 mm, with a window for the infrared sensor; joined to the PVC tubes by push-fit couplings secured with printed clamp sleeves |
| **IR clip** | TCRT5000 module looking through the window into the ball path, under a small hood |
| **Upper tube** | 25 mm PVC conduit, 300 mm |
| **Power module** | box clamped to the tube: 4 × AA cells, fuse, circuit breaker, main switch, diode and capacitors |
| **Handle** | clamp frame with the grip bar (TPU sleeve), the touch pad on top and the housing for breadboard, ESP32, OLED and LEDs |
| **Metering module** | housing on the top end of the tube with the SG90 servo, the metering wheel and a clear acrylic cover |
| **Magazine** | clear tube with about 10 seed balls and a cap |

## The planting cycle

1. **Push in.** The depth collar stops the stick: the tip is always 30 mm deep, and every seed ball ends up
   28 mm deep (12 mm of soil over a 16 mm ball).
2. **Read the moisture.** A capacitive sensor measures the soil around the blade. The firmware averages the reading,
   converts it to a percentage between the calibrated "air" and "water" values and shows a band: dry (below 30 %),
   suitable (30–70 %, green LED) or wet (above 70 %).
3. **Lift about 2 cm.** This frees the jaws in the soil so they can open.
4. **Tap.** The servo turns the metering wheel by 180° (about 0.3 s). The wheel has two pockets: one is always open
   to the magazine and takes the next ball; the other faces down and releases one ball into the tube.
5. **Count.** The ball falls about 0.4 m past the infrared sensor in the sight tube. The firmware waits up to 1.5 s
   for the falling edge; then it counts the ball, flashes the red LED twice and writes a log line.
   If nothing passes, the display says "no ball" (check the magazine).
6. **Open the jaws.** The ball hits the inside of the tip and pushes the two jaws apart against the O-ring.
   The O-ring is soft enough that the ball's fall opens them, and strong enough to keep the tip closed while
   pushing it into the soil.
7. **Next spot.** Pull the stick out; the jaws close again.

## Electronics

- **ESP32** dev board on a breadboard in the handle housing.
- **Power:** 4 × AA NiMH (4.8 V). A slow-blow T2A fuse and a 10 A blade circuit breaker protect the wiring, a rocker
  switch turns everything off. The servo gets the battery voltage directly; the ESP32 is fed through a Schottky diode,
  so USB never powers the servo and the servo's start-up current does not reset the ESP32 (1000 µF and 470 µF
  buffer capacitors).
- **Sensors:** capacitive moisture sensor (analogue, GPIO 34), TCRT5000 infrared module (digital, GPIO 19, with
  interrupt), battery voltage via a 100 k / 100 k divider (GPIO 35).
- **User interface:** TTP223 touch pad (tap, hold 2 s, hold 5 s), 0.96" OLED (I²C), green and red LED.
- **Data:** every ball is logged to flash (`log.csv`) with moisture, band, result and supply voltage. With Wi-Fi on,
  a phone can open the live page, download the log and use the web console.

Pin map and wiring: [electronics/wiring.md](../electronics/wiring.md). Firmware details:
[firmware/README.md](../firmware/README.md).

## Seed balls

Seed balls are made from clay, compost and seeds in the printed mould and dried; the go/no-go gauge sorts them to
Ø 15.5 ± 0.5 mm, the size the ball path is designed for (a 15.5 mm ball has at least 4.5 mm clearance in the tubes and the
foot, 1.9 mm in the wheel pocket). For demonstrations, 3D-printed dummy balls with the same size and weight work too.

Why it is built this way, and what we learned: [design-decisions.md](design-decisions.md).
