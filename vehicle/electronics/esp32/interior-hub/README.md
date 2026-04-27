# ESP32-S3 Interior Hub

**Role:** Listens to Uno R4 serial events, controls interior lighting zones, and forwards scene changes to the exterior node and IRIS display via wired UART.

## Status: Firmware complete — not yet wired or bench-tested

Port from QT Py RP2040 is done. Pin assignments are set in `interior-hub.ino` but have not been physically verified against the Hosyond board.

Open before first power-on:
- Confirm pin assignments against physical Hosyond ESP32-S3 N16R8 board and wiring
- Bench test: Serial1 receives `SCENE_NEXT` from Uno → dashboard strips glitter, Serial2 TX sends scene index

## Target hardware
- Hosyond ESP32-S3 N16R8 (16MB flash, 8MB PSRAM)
- Powered by battery USB-C 18W (via 5V PD trigger board)
- Board target: `esp32:esp32:esp32s3`

## Pin assignments (set — confirm before wiring)

| Pin constant | GPIO | Role |
|---|---|---|
| PIN_SERIAL_RX | 16 | Serial1 RX ← Uno R4 TX |
| PIN_SERIAL_TX | 17 | Serial1 TX (unused, required by begin()) |
| PIN_BROADCAST_TX | 18 | Serial2 TX → exterior node + IRIS (shared line) |
| PIN_DASH | 5 | Dashboard WS2812B strip |
| PIN_MATRIX | 6 | Back window SK6812 8x8 matrix |
## UART topology

| UART | Direction | Role |
|---|---|---|
| Serial (USB) | Debug out | `Serial.println()` debug only |
| Serial1 (RX=16) | Uno R4 → Hub | Receives event strings at 9600 baud |
| Serial2 (TX=18) | Hub → Ext + IRIS | Broadcasts scene index integer, shared wire |

## Pin summary
See [baja-lighting-spec.md](../../baja-lighting-spec.md) for full hardware map and zone table.
