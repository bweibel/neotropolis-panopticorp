# Hosyond ESP32-S3 N16R8 — Exterior Node

**Role:** Receives scene index from Interior Hub via wired UART; drives side strips, hood scoop, and accent elements with glitter animation.

Top scanner (sensor bar) is a separate node — see `esp32/sensor-bar/`.

## Status: Pin constants updated — not yet wired or bench-tested

Logic is complete. Pin assignments updated for N16R8 (avoiding OPI PSRAM range 26–37). Wheel well PWM pins assigned but unverified — stretch goal.

## Target hardware
- Hosyond ESP32-S3 N16R8 (16MB flash, 8MB OPI PSRAM)
- Powered by battery USB-C 100W (via 5V PD trigger board)
- Board target: `esp32:esp32:esp32s3:PSRAM=opi,CDCOnBoot=cdc`

## Pin assignments

| Constant | GPIO | Role |
|---|---|---|
| PIN_SIDE_LEFT | 13 | Side strip left (WS2812B, 150 LEDs) |
| PIN_SIDE_RIGHT | 14 | Side strip right (WS2812B, 150 LEDs) |
| PIN_HOOD_SCOOP | 15 | Hood scoop (WS2812B, ~8 LEDs) |
| PIN_ACCENT_1 | 4 | Accent element 1 (WS2812B, ~8 LEDs) |
| PIN_ACCENT_2 | 2 | Accent element 2 (WS2812B, ~8 LEDs) |
| PIN_HUB_RX | 16 | Serial2 RX ← Hub Serial2 TX |
| PIN_HUB_TX | 17 | Serial2 TX (unused) |
| Wheel well pins | 18–25, 38–44 | MOSFET PWM stretch goal — verify before wiring |

## Upload
```sh
arduino-cli compile --fqbn esp32:esp32:esp32s3:PSRAM=opi,CDCOnBoot=cdc ExteriorNode/
arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:esp32s3:PSRAM=opi,CDCOnBoot=cdc ExteriorNode/
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```

## Before running
- Adjust `HOOD_NUM_LEDS` and `ACCENT_NUM_LEDS` to actual strip lengths
- Confirm wheel well MOSFET polarity (common cathode assumed) before wiring

## Pin summary
See [baja-lighting-spec.md](../../baja-lighting-spec.md) for full hardware map.
