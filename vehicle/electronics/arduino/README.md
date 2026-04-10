# Arduino — Uno R4 WiFi

**Role:** Head unit controller and serial broadcaster.

Reads physical inputs (rotary encoder, buttons, toggle switches), controls the Pioneer head unit via an X9C104 digital potentiometer, and broadcasts one-way serial events to the ESP32 Interior Hub.

## Target hardware
- Arduino Uno R4 WiFi
- Powered by car 12V (ignition-switched)

## Upload
Use Arduino IDE or `arduino-cli` targeting `arduino:renesas_uno:unor4wifi`.

## Before building
Paste the following into `PioneerController/`:
- `PioneerController-ProtoThread.ino` body (existing sketch)
- `X9C.h` / `X9C.cpp` (Phil Bowles, MIT)
- `pt.h`, `lc.h`, `lc-switch.h`, `lc-addrlabels.h`, `pt-sem.h` (Protothreads, Adam Dunkels)

Fix the two off-by-one bugs in the queue functions before adding new code (see TODOs in sketch).

## Pin summary
See [baja-lighting-spec.md](../baja-lighting-spec.md) for the full hardware map and pin table.
