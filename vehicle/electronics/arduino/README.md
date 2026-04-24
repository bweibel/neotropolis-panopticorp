# Arduino — Uno R4 WiFi

**Role:** Head unit controller and serial broadcaster.

Reads physical inputs (rotary encoder, buttons, toggle switches), controls the Pioneer head unit via an X9C104 digital potentiometer, and broadcasts one-way serial events to the ESP32 Interior Hub.

## Status: Beta — basic functionality tested complete

All core features implemented and bench-tested:
- Rotary encoder volume control + mute
- Skip forward/back buttons → Pioneer head unit
- Scene cycle buttons → serial broadcast to Interior Hub
- T2 LED master toggle
- Character LCD state machine (boot sequence, idle, event messages)
- LED matrix animations (boot drama, track chevron sweep)
- OTA update support

**Remaining:** polish passes + physical installation into the Baja.

Open item: `ledMasterOn` hardcoded to `true` in `setup()` — update to read T2 toggle pin state once the physical toggle is wired.

## Target hardware
- Arduino Uno R4 WiFi
- Powered by car 12V (ignition-switched)

## Upload
Uses `arduino-cli` targeting `arduino:renesas_uno:unor4wifi`.

```sh
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi PioneerController/
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:renesas_uno:unor4wifi PioneerController/
```

OTA upload available once WiFi credentials are set in `arduino_secrets.h`.

## Pin summary
See [baja-lighting-spec.md](../baja-lighting-spec.md) for the full hardware map and pin table.
