# ESP32 Exterior Node

**Role:** Receives scene changes from the interior hub via ESP-NOW and controls all exterior lighting zones.

## Target hardware
- ESP32 (with camera module — camera not used in MVP)
- Powered by battery USB-C 100W (via 5V PD trigger board)

## Upload
Use Arduino IDE or `arduino-cli` with the ESP32 board package (`esp32:esp32:esp32s3`).

## Before running
- Flash this device first, then note its MAC address and set `EXTERIOR_NODE_MAC` in `interior-hub.ino`
- Confirm pin assignments against physical wiring
- Adjust `HOOD_NUM_LEDS` and `ACCENT_NUM_LEDS` to actual strip lengths

## Pin summary
See [baja-lighting-spec.md](../../baja-lighting-spec.md) for the full hardware map and zone table.
