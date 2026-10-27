# Design decisions and lessons learned

The main engineering choices behind SeedStick, and what we learned along the way.

## Mechanism

**One ball per tap: a metering wheel with two pockets.** The wheel sits under the magazine and has two opposite
pockets. A 180° turn of the SG90 servo brings the loaded pocket from the magazine to the tube and the empty one back
under the magazine, so every turn releases exactly one ball. The servo only gets pulses for 0.6 s per dose and is
then switched off: no humming and less current. Both end positions are set once and stored in flash.

**A self-closing tip.** A plain open tube would fill with soil when it is pushed in. The tip is a closed cone made of
two jaws on M2 pivots, held shut by a small O-ring. The falling seed ball pushes the jaws apart and drops into the
hole; when the stick is pulled out, the ring closes them again.
- *Lesson:* our first jaws had no edge below the ring, and the ring slid towards the tip. The final jaws have a
  seat with a lip that keeps the ring in place.
- *Lesson:* the ring must be soft. A soft silicone ring needs only about a third of the ball's fall energy to open
  the jaws; a typical hard NBR ring would need most of it.
- *Usage:* lift the stick about 2 cm before tapping, so the jaws have room to open in the soil.

**Same depth every time.** An Ø 80 mm collar 30 mm above the tip stops the stick at the surface, so the ball always
lands at the same depth instead of depending on how hard someone pushes.

**Standard tubes, printed joints.** The body is 25 mm PVC electrical conduit and a clear polycarbonate tube from the
hardware store. Push-fit couplings join them; printed clamp sleeves around the couplings take the pull when the stick
comes out of the soil (designed for 100 N pull and 200 N push). Every module clamps onto the tube, so parts can be
moved, replaced or reprinted without taking the stick apart.

**Printable without supports.** All printed parts except the power box print without supports (overhangs at most
45°, bridges at most 10 mm apart from one 17.4 mm bridge in the metering wheel). Every screwed joint uses brass heat-set inserts, so the parts survive repeated assembly.
A fit-test card lets you check tube fits, nut pockets and insert holes on your printer before printing the real parts.

## Electronics

**Safe power first.** The rules required less than 12 V and 10 A, a fuse and a circuit breaker. We chose 4 × AA NiMH
(4.8 V) over LiPo, a slow-blow T2A glass fuse, a 10 A blade circuit breaker and one main switch. The servo takes the
battery voltage directly; the ESP32 is fed through a Schottky diode with buffer capacitors, so the servo's start-up
current does not reset the controller and USB never powers the servo.
- *Lesson:* a laptop USB port was damaged while we worked on the electronics. The likely path is the battery feeding
  back into USB when the main switch is on. Since then: main switch off before plugging in USB, and firmware 1.2 has
  a web console over Wi-Fi so set-up needs no cable at all.

**Count, don't assume.** A TCRT5000 infrared module looks through a window in the clear tube. The firmware starts a
1.5 s window with every dose and counts the falling edge in an interrupt; only a confirmed ball is counted and logged,
otherwise the display says that no ball came.

**Capacitive moisture sensing.** A capacitive sensor does not corrode like a resistive one. It is calibrated once in
air and once in water; the firmware maps the reading to a percentage and to three bands (dry below 30 %, suitable,
wet above 70 %).
- *Lesson:* a constant reading of about 140 mV meant no signal at all. It was a loose contact at the sensor plug: a
  connector that only works while you press it fails in the field. Solder joints with heat shrink and strain relief
  fixed it.

**Breadboard instead of a PCB.** For a six-week build, a half-size breadboard in the handle housing was faster to
change than a custom board. Four flat wire bridges under the ESP32 reach the pins it covers. A soldered perma-proto
board was planned as the fallback, should the breadboard not survive the shock of pushing the stick into the soil.

## Software

- **One button.** Tap, hold 2 s and hold 5 s cover planting, Wi-Fi and calibration; the display shows what releasing
  the finger will do before it happens.
- **Self-test.** The command `test` walks through every part (display, LEDs, touch, IR, moisture, supply, servo) and
  reports what is missing, so a fault can be found without opening the stick.
- **Data you can take away.** Every ball is a line in `log.csv` with time, moisture, band, result and battery voltage;
  a phone can download it from the stick's own access point.

## Showcase stand

Our first idea held the stick by a cone around the foot. It could not work: the cone would have been narrower than
the Ø 80 mm collar above it, so the stick could never go in. The final stand works like a guitar stand: the collar
sits in a cup, and the stick leans 3° back into a fork on a post. A ballast tray (about 1 kg) keeps it from tipping.
