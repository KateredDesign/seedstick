# Assembly

Build order in short. Sides on the stick are named from the user's view: **front** = display, **right** = moisture
sensor, sight window and grip, **left** = power module.

## 1. Prepare

- **Printed parts:** remove stringing and brims, check that every hole and slot is free
  ([hardware/print-settings.md](../hardware/print-settings.md)).
- **Tubes:**
  - PVC conduit 25 mm: lower tube **554 mm**, upper tube **300 mm**, square cuts, without the flared end.
  - Polycarbonate tube 25 × 22 mm: sight tube **140 mm**, magazine tube **155 mm**.
  - Deburr every end inside and outside; chamfer the inside of the lower tube's top end (0.5 × 45°), where the ball
    passes from the clear tube into the PVC tube.
  - **Window** in the sight tube for the infrared sensor: 13 × 14 mm, centre 68.9 mm above the bottom end; drill
    4 × Ø 6.5 mm with a wooden dowel inside as backing, saw out the webs, file and deburr. Only one wall: the
    opposite wall is the ball path.
  - Pencil marks 25.5 mm from the tube ends that go into a coupling (so each 51 mm coupling sits centred).
- **Heat-set inserts** with the soldering iron, flush and straight: metering housing 4 × M3 (cover) and 2 × M2
  (servo cover), front panel 2 × M2, power box 4 × M3.
- **Long lead times:** glue the servo horn into the metering wheel with 2-part epoxy (cure 12 h); seal the cut edges
  of the moisture sensor blade with nail polish or epoxy.

## 2. Sub-assemblies on the bench

1. **Foot:** M4 nut into the lug; slide both jaws onto the tongues and fix each with an M2 × 12 (self-tapping, the
   jaw must swing freely); push the **O-ring 22 × 1** over the tip into its seat above the lip.
2. **Sensor holder:** slide the moisture sensor board in from below until it rests against the stops.
3. **IR clip:** TCRT5000 module into the pocket, 2 × M2.5 × 6, hand-tight.
4. **Metering module:** servo from the servo side into the housing (the two long screws from the servo bag, at most
   11 mm under the head; no tip may stick into the pot), servo cable into the notch, servo cover with 2 × M2 × 6. The wheel follows after the servo has been set to
   an end position (step 4).
5. **Handle frame:** M2.5 nuts into the panel bosses, grip sleeve over the grip bar, bar into the head, housing
   onto the bar.

## 3. Build the stick from the bottom up

1. **Foot** onto the lower tube (40 mm socket), tighten the M4 by hand.
2. **Sensor holder** around the foot: its blade slides through the slot in the collar; 2 × M4 × 20.
3. **Coupling A** on the lower tube, **sight tube** (window to the right) into it, **clamp sleeve A** around it:
   nuts first, 4 × M4 × 20 crosswise until both joints are equally wide (about 0.8 mm stays open).
4. **IR clip** on the window, nose straight into the window; hood over the plug; two cable ties.
5. **Coupling B**, **upper tube**, **clamp sleeve B** like step 3 (put the nuts in before the power box comes).
6. **Power box** from the top onto the upper tube, 3 mm above clamp B, clamp from the right.
7. **Handle frame** from the top, 1 mm above the power box; grip to the right, housing opening to the front.
8. **Metering module** onto the top end of the tube (32 mm socket), servo cover to the right.
9. **Magazine tube** and **cap**.
10. **Electronics:** cables into the housing, breadboard in, everything plugged in after
    [electronics/wiring.md](../electronics/wiring.md); battery cells and lid; OLED, LEDs and front panel.

Cables run on the outside of the tube with cable ties or tape, never under a clamp ring.

## 4. Set up and test

1. **Metering wheel:** move the servo to end position A (firmware command `a`), put the wheel on so that one pocket
   points straight up, screw the horn; adjust A and B with `+`/`-` until a pocket is vertical in both, `save`.
   Then the acrylic cover with 4 × M3 × 8, hand-tight.
2. **Infrared sensor:** tap 10 times with balls in the magazine; turn the potentiometer until every ball is counted.
3. **Moisture:** hold the touch pad 5 s: foot in the air, tap; foot in water up to the line, tap.
4. **Soil test:** dry, moist and wet soil must show the right band.
5. **Endurance:** 20 balls in a row without a miss or a restart.
6. Transport cap on the tip, USB plug in the housing opening.
