# The UNSW Build-a-thon 2026

SeedStick was built for the **UNSW Build-a-thon 2026**, a six-week mechatronics competition run by the
**UNSW Robotics and Mechatronics Society (RAMSoc)** and **UNSW Women in Technology (WIT)**, with
**UNSW Founders**. Teams of two to six students design, build and program a working prototype that answers a brief,
and pitch it to university and industry judges at a final showcase.

## Result

- 🏆 **1st place** out of 60 teams (about 300 participants), chosen by the judging panel.
- 💚 **People's Choice Award**: voted the favourite project by the visitors and participants of the final showcase.

## Format

| Week | What happened |
|---|---|
| 1 | Kick-off: the brief is released, teams form and register |
| 2 | CAD workshops |
| 3 | ESP32 and PCB workshops; the component shop opens |
| 4–5 | Build sessions at the UNSW makerspaces, with mentors |
| 6 | Build sessions, poster submission, **final showcase** at the Prince Henry Centre (23 October 2026) |

- **Components:** a base kit (ESP32, breadboard, wires, LEDs, resistors) plus a shop that trades tiered tickets
  (S, A, B, C) for parts such as the OLED, servo, IR sensor and battery holder.
- **Budget:** up to AUD 150 for additional purchased parts, declared with market prices incl. GST.
- **Showcase:** a physical prototype, an A2 poster and a live pitch at the team's booth.

## The brief

The brief framed three problems around the environment: **monitoring** ("we can't protect what we can't see"),
**repurposing** (waste that is never reused) and **restoration**. We chose **problem 3, restoration**: countries have pledged to restore 1.2 billion hectares of degraded land by
2030, but only about a tenth of that area is under restoration, because the work is still done by hand.

A solution had to restore degraded environments by doing at least one of these: reduce the labour required, reduce
harmful by-products, rebuild the ecosystem structure, or improve germination and survival rates.
SeedStick targets **labour** (plant standing up, one push and one tap per spot) and **germination** (every seed ball
at the same depth, and a moisture check at every spot before it goes in).

## Rules that shaped the design

| Rule | How SeedStick meets it |
|---|---|
| Must be a mechatronic system: mechanical, electrical and software parts | dosing mechanism and jaws, sensors and power module, ESP32 firmware with logging and Wi-Fi |
| Below 12 V and at most 10 A; fuses **and** circuit breakers are mandatory | 4.8 V NiMH pack, about 1.2 A peak; T2A glass fuse plus a 10 A blade circuit breaker, one main switch |
| Sharp edges enclosed | servo under a printed guard, metering wheel behind an acrylic cover, transport cap on the tip |
| No hazardous materials; LiPo rules | NiMH cells instead of LiPo |
| AUD 150 budget for purchased parts | about AUD 53 of purchased parts plus filament ([bill of materials](../hardware/bom.md)) |
| A2 poster and physical prototype at the showcase | [poster](poster/SeedStick_Poster_A2.pdf), working prototype, showcase stand |

## Judging criteria

| Criterion | Weight | What we showed |
|---|---|---|
| Creativity | 15 % | seed balls dosed one at a time by a metering wheel, a self-closing jaw tip that the falling ball opens |
| Technical execution | 40 % | printed mechanism designed for 100 N pull and 200 N push, fused power module with diode isolation, firmware with self-test, IR confirmation, log and web console |
| Design | 10 % | depth stop instead of judging depth by eye, grip with the touch pad on the handle, display and LEDs facing the user, printed parts in one consistent style |
| Sustainability and solution | 20 % | answers the restoration brief: less labour per seedling spot, consistent depth, soil check, proof of work |
| Presentation | 15 % | A2 poster, interactive figure of the planting cycle, live demonstration at the booth |
