# Print settings

All parts were printed on a Bambu Lab X1 Carbon with a 0.4 mm nozzle. Every STL file already lies in its print
orientation: load it as it is, do not let the slicer re-orient it ("arrange" is fine).

## General settings

| Setting | Value |
|---|---|
| Layer height | 0.2 mm |
| Walls | 3; **5** for the load-bearing parts (marked **L** below) |
| Infill | 15 % grid; **25 % gyroid** for the load-bearing parts |
| Supports | **off** for every part except the power box |
| Brim | **none**, except the handle bar, the coupling clamps and the power box (5 mm) |
| PETG build plate | textured PEI, or smooth PEI with glue stick as a release agent |
| Bridges | default (the metering wheel has a 17.4 mm bridge) |

## Parts of the stick (`stl/device/`)

| File | Qty | Material | Load | Orientation on the bed | approx. g |
|---|---|---|---|---|---|
| `foot.stl` | 1 | PETG | L | tip up, tube socket and lug on the bed | 49 |
| `foot-jaw.stl` | 2 | PETG | L | on its top face, tip up | 3 |
| `transport-cap.stl` | 1 | TPU 95A | | as in the file | 15 |
| `sensor-holder-front.stl` | 1 | PETG | | ring down | 11 |
| `sensor-holder-back.stl` | 1 | PETG | | ring down | 5 |
| `coupling-clamp-left.stl` | 2 | PETG | L, brim 5 mm | standing, lower ring on the bed | 18 |
| `coupling-clamp-right.stl` | 2 | PETG | L, brim 5 mm | standing, lower ring on the bed | 18 |
| `ir-clip-front.stl` | 1 | PETG | | standing | 6 |
| `ir-clip-back.stl` | 1 | PETG | | standing | 4 |
| `ir-hood.stl` | 1 | PETG | | as in the file | 8 |
| `metering-housing.stl` | 1 | PETG | | servo wall down, pot open to the top | 51 |
| `metering-wheel.stl` | 1 | PLA, white | | flat, horn side up | 16 |
| `servo-cover.stl` | 1 | PETG | | top face down | 8 |
| `magazine-cap.stl` | 1 | PETG | | top face down, like a cup | 10 |
| `handle-housing.stl` | 1 | PETG | L | on its back, opening up | 102 |
| `handle-clamp-cap.stl` | 1 | PETG | L | tube axis vertical | 9 |
| `handle-head.stl` | 1 | PETG | L | tube axis vertical | 29 |
| `handle-bar.stl` | 1 | PETG | L, brim 5 mm | standing on the thick end | 20 |
| `grip-sleeve.stl` | 1 | TPU 95A | | as in the file | 22 |
| `front-panel.stl` | 1 | PETG | | visible face down | 28 |
| `oled-bracket.stl` | 1 | PETG | | flat, pads up | 1 |
| `power-box.stl` | 1 | PETG | L, brim 5 mm, **tree supports on** | standing, ring to the right | 69 |
| `power-box-lid.stl` | 1 | PETG | | outer face down | 24 |
| `usb-plug.stl` | 1 | TPU 95A | | collar down | 1 |

Together about 565 g: roughly 510 g PETG, 16 g PLA and 38 g TPU. The gram values are estimates from the models;
the slicer shows the exact numbers.

## Accessories (`stl/accessories/`)

| File | Qty | Material | Notes |
|---|---|---|---|
| `showcase-stand.stl` | 1 | PETG | Ø 210 mm, about 255 g and 9 h. No supports: the fork has a 45° underside. Fill about 1 kg of ballast (sand in a bag) into the ring tray. Set the stick in tilted forward, foot into the cup, then lean it back into the fork. |
| `dummy-ball-half.stl` | 24 | PETG, **light colour** | Two halves make one Ø 15.5 mm ball. **100 % infill**, so a ball weighs about 2.5 g like a dried seed ball and still pushes the jaws open. Align the halves with two 4.6 mm pieces of 1.75 mm filament, glue with cyanoacrylate. Not black: the infrared counter must see them. |
| `dummy-ball-plate-24.stl` | 1 | as above | all 24 halves on one plate (113 × 74 mm) |

## Calibration (`stl/calibration/`)

| File | Qty | Material | Notes |
|---|---|---|---|
| `fit-test-card.stl` | 1 | PETG | test rings, nut pockets, insert holes and servo frame: print first to check the fits of your printer |
| `seed-ball-mould-half.stl` | 2 | PLA | two identical halves; three cavity sizes to find the drying shrinkage of your clay mix |
| `seed-ball-gauge.stl` | 1 | PLA | go hole 16 mm, no-go hole 15 mm: a good ball passes 16 and stops at 15 |

## Laser part (`laser/`)

| File | Material | Notes |
|---|---|---|
| `sight-cover.dxf` | acrylic (PMMA) 3 mm, clear | Ø 66 mm window over the metering wheel, 4 × M3 holes |
