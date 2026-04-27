# XIAO ESP32-S3 Sense — Sensor Bar

**Role:** Drives the top scanner strip (150 WS2812B LEDs) with mic-driven audio reactivity. Scene-aware: strip off in scene 0, audio reactive in scene color palette for scenes 1 and 2. Color scheme is independent — not locked to glitter animation used by other exterior zones.

## Status: Not started — firmware to be written

## Target hardware
- Seeed Studio XIAO ESP32-S3 Sense (8MB flash, 8MB PSRAM, PDM mic onboard)
- Powered by battery USB-C 100W (shared rail with exterior node)
- Board target: `esp32:esp32:XIAO_ESP32S3`

## Pin assignments

| Constant | GPIO | XIAO label | Role |
|---|---|---|---|
| PIN_SCENE_RX | 44 | D7 | Serial1 RX ← Hub Serial2 TX |
| PIN_STRIP | 1 | D0 | WS2812B data out (150 LEDs) |
| PDM_CLK | 42 | onboard | PDM mic clock (onboard) |
| PDM_DATA | 41 | onboard | PDM mic data (onboard) |

## Serial input
Taps the same Hub Serial2 TX shared line as the exterior node and IRIS display.
- Baud: 9600
- Format: `"0\n"`, `"1\n"`, `"2\n"` — scene index

## Behaviour
- **Scene 0 (OFF):** strip off regardless of audio
- **Scene 1 (RED):** audio reactive, red palette
- **Scene 2 (GREEN):** audio reactive, green palette

## Visual effect: amplitude pulse

Strip brightness rises and falls with audio amplitude. All 150 LEDs show the scene color at a brightness level driven by an envelope follower on the PDM mic input. Fast attack, slow decay — keeps the strip visually alive without strobing. This is the initial test effect; more complex effects (center burst, VU bar) are future iterations.

## Open items
- [ ] Firmware not yet written — see `sensor-bar-impl-spec.md`
- [ ] Confirm 150 LED count matches physical strip
- [ ] Confirm PDM mic GPIO numbers against XIAO Sense schematic (41/42 assumed)
- [ ] Tune envelope attack/decay and amplitude scaling on hardware
