# QT Py RP2040 Interior Hub

**Role:** Listens to Uno R4 serial events, controls interior lighting zones, and forwards scene changes to the exterior node via wired UART.

## Target hardware
- Adafruit QT Py RP2040 (product 4900)
- Powered by battery USB-C 18W (via 5V PD trigger board)

## Upload
Use Arduino IDE or `arduino-cli` with the arduino-pico core (Earle Philhower), board target: `rp2040:rp2040:adafruit_qtpy_rp2040`.

## Before running
- Confirm pin assignments against physical wiring

## Pin summary
See [baja-lighting-spec.md](../../baja-lighting-spec.md) for the full hardware map and zone table.
