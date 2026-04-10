# ESP32 Interior Hub

**Role:** Listens to Uno R4 serial events, controls interior lighting zones, and forwards scene changes to the exterior node via ESP-NOW.

## Target hardware
- ESP32 (no camera module)
- Powered by battery USB-C 18W (via 5V PD trigger board)

## Upload
Use Arduino IDE or `arduino-cli` with the ESP32 board package (`esp32:esp32:esp32`).

## Before running
- Set `EXTERIOR_NODE_MAC` to the actual MAC address of the exterior node ESP32
- Confirm pin assignments against physical wiring

## Pin summary
See [baja-lighting-spec.md](../../baja-lighting-spec.md) for the full hardware map and zone table.
