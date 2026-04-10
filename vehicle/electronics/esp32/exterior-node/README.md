# ESP32-S3 Exterior Node

**Role:** Receives scene index via wired serial (UART) from the RP2040 interior hub and controls all exterior lighting zones.

## Target hardware
- ESP32-S3 (with camera module — camera not used in MVP)
- Powered by battery USB-C 100W (via 5V PD trigger board)

## Upload
Use Arduino IDE or `arduino-cli` with the ESP32 board package (`esp32:esp32:esp32s3`).

## Before running
- Confirm pin assignments against physical wiring
- Adjust `HOOD_NUM_LEDS` and `ACCENT_NUM_LEDS` to actual strip lengths

## Pin summary
See [baja-lighting-spec.md](../../baja-lighting-spec.md) for the full hardware map and zone table.
