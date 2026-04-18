# ESP32-S3 Interior Hub

**Role:** Listens to Uno R4 serial events, controls interior lighting zones, and forwards scene changes to the exterior node and IRIS display via wired UART.

## Target hardware
- Hosyond ESP32-S3 N16R8 (16MB flash, 8MB PSRAM)
- Powered by battery USB-C 18W (via 5V PD trigger board)

## Upload
Use Arduino IDE or `arduino-cli` with the ESP32 board package (`esp32:esp32:esp32s3`).

## Before running
- Confirm pin assignments against physical wiring
- Note: sketch is being ported from QT Py RP2040 — pin constants will change

## Pin summary
See [baja-lighting-spec.md](../../baja-lighting-spec.md) for the full hardware map and zone table.
