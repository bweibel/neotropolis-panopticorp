# Baja Lighting Controller: Implementation Spec

## Purpose

This document is a handoff spec for Claude Code. It defines the full software architecture for the Baja AV and lighting controller system. Hardware selection and architecture decisions are finalized. Implementation should follow this spec without revisiting those decisions.

---

## System Overview

Three independent software systems sharing one-way serial links.

**Permanent system** (Arduino Uno R4 WiFi, car 12V ignition-switched):
- Reads physical inputs (encoder, buttons, toggles)
- Controls Pioneer head unit via X9C104 digital pot
- Broadcasts serial events regardless of whether anything is listening

**Removable lighting module** (battery powered):
- ESP32-S3 N16R8 Interior Hub: listens to Uno serial, controls interior strips, sends scene index to exterior node and IRIS display via wired UART
- ESP32-S3 N16R8 Exterior Node: receives scene index via wired UART from hub, controls side strips, hood scoop, and accent elements
- XIAO ESP32-S3 Sense Sensor Bar: taps hub Serial2 TX for scene index; drives top scanner (150 LEDs) with mic-driven audio reactivity in scene color palette

**Permanent display module** (car 12V):
- XIAO ESP32-S3 Sense — IRIS display character
- Receives scene index via wired UART from interior hub (shared Serial2 TX)
- Drives 1.28" round GC9A01 TFT (240×240) via hardware SPI — animated eye lens
- Handles all display animation independently
- WH1602B-TMI-JT 16×2 character LCD — driven by Uno R4 directly
- Shows scene name, volume level, boot messages, and Panopticorp flavor text

---

## Hardware Map

| Device | Role | Connection |
|---|---|---|
| Arduino Uno R4 WiFi | Head unit controller, input surface | Car 12V |
| ESP32-S3 N16R8 (Hosyond) | Interior lighting hub | Battery 18W USB-C |
| ESP32-S3 N16R8 (Hosyond) | Exterior lighting node | Battery 100W USB-C |
| XIAO ESP32-S3 Sense (Seeed) | Sensor bar (top scanner) — audio reactive | Battery 100W USB-C |
| ESP32-S3 N16R8 (Hosyond) | IRIS display character | Car 12V (permanent) |
| WH1602B-TMI-JT 16×2 LCD | Status + flavor text display | Uno R4 (permanent, 5V) |
| Pixelblaze V3 Standard | Top scanner + rear window strips | Battery (via exterior node power rail) |

---

## Physical Inputs (Uno R4)

### Toggle Switches (between seats, seat heater location)

| Toggle | Function | MCU involvement |
|---|---|---|
| T1 | Offroad lights / lightbars (12V relay) | None, fully independent |
| T2 | LED master power (cuts 12V to lighting module) | Uno reads state on boot |

Toggle T2 is read by the Uno on boot and on change. When OFF, Uno suppresses SCENE_* events. All other events (VOL, SKIP, MUTE) are always emitted.

### Pioneer Head Unit Integration

> ⚠️ **NEEDS REVIEW** — Resistance values in `PioneerController` (`REST_VOLUMEUP`, `REST_VOLUMEDOWN`, etc.) were empirically tuned but the target Pioneer model is not documented. Possible code/hardware mismatch. Verify resistance map against the actual installed unit before relying on this integration. Deprioritised — revisit if time permits before event.

### Center Console Panel (3D printed mount, Uno R4 visible)

| Input | Pin (assign as constant) | Function |
|---|---|---|
| Rotary encoder CLK | ENC_CLK | Volume up/down |
| Rotary encoder DT | ENC_DT | Volume direction |
| Rotary encoder SW | ENC_SW | Mute toggle (single press) |
| Button 1 | BTN_1 | Skip back |
| Button 2 | BTN_2 | Skip forward |
| Button 3 | BTN_3 | Scene cycle forward |
| Button 4 | BTN_4 | Scene cycle backward |
| Toggle T2 | TOGGLE_LED | LED master power state |

All pins defined as named constants at top of sketch. No magic numbers.

Button keycap labels (Panopticorp theme): SCAN, SWEEP, PURGE, UPLINK (or equivalent, cosmetic only).

---

## Serial Event Schema

### Transport

- SoftwareSerial on Uno R4 (pins to be assigned, define as constants)
- Baud: 9600
- Format: ASCII string, newline terminated (`\n`)
- Direction: Uno R4 -> ESP32 Interior Hub only (one-way)
- No handshake, no acknowledgment, no flow control

### Event Strings

| Event | Trigger | Suppressed when LED master off |
|---|---|---|
| `VOL_UP` | Encoder turn clockwise | No |
| `VOL_DOWN` | Encoder turn counter-clockwise | No |
| `MUTE` | Encoder button press | No |
| `SKIP_FWD` | Button 1 press | No |
| `SKIP_BACK` | Button 2 press | No |
| `SCENE_NEXT` | Button 3 press | Yes |
| `SCENE_PREV` | Button 4 press | Yes |

Future events can be added without breaking existing receivers (ESP32 ignores unknown strings).

---

## Scene Definitions

Scene state is an integer 0-2, cycling on each `SCENE_NEXT` event.

| Scene index | Name | Color | Animation |
|---|---|---|---|
| 0 | OFF | All off | None |
| 1 | RED | #CC0000 (warm red, tune to taste) | Glitter (see below) |
| 2 | GREEN | #00CC00 (tune to taste) | Glitter (see below) |

### Glitter Animation

Applied identically to all active WS2812B zones when scene is RED or GREEN.

- Base color: scene color at full brightness
- Each loop iteration: select N random pixels (N = strip_length * 0.05, i.e. 5%)
- For each selected pixel: randomly set brightness to 60-100% of base color
- All other pixels: hold at 85% brightness
- No blocking delays, runs in main loop

This produces a subtle shimmer without strobing. Adjust percentages during testing.

---

## Hub Broadcast Link (Serial2 TX — shared)

- Transport: Wired UART, one-way TX from Interior Hub Serial2
- Baud: 9600
- Format: ASCII integer, newline terminated (`"0\n"`, `"1\n"`, `"2\n"`)
- Content: Scene index only
- No handshake, no acknowledgment
- **Three receivers on one TX line:** Exterior Node N16R8, Sensor Bar XIAO, IRIS Display N16R8

---

## Lighting Zones and Controllers

### Interior Hub (ESP32 #1)

| Zone | Type | LEDs | Control method | Notes |
|---|---|---|---|---|
| Dashboard outline | WS2812B 2.7mm | 160 | FastLED, single data pin | |
| Back window matrix | SK6812 8x8 (Electromage) | 64 | FastLED, single data pin | Glitter animation; RGBW, W unused |

### Exterior Node (ESP32-S3 N16R8)

| Zone | Type | LEDs | Control method | Notes |
|---|---|---|---|---|
| Side strips x2 | WS2812B | 150 each | FastLED, two data pins | Glitter animation |
| Hood scoop | WS2812B | <10 | FastLED | Glitter animation |
| Accent elements x2 | WS2812B | ~8 each | FastLED | Glitter animation |
| Wheel wells x4 | Static 4-line RGB | unknown | MOSFET PWM (stretch goal) | Can run standalone if cut |

### Sensor Bar (XIAO ESP32-S3 Sense)

| Zone | Type | LEDs | Control method | Notes |
|---|---|---|---|---|
| Top scanner | WS2812B | 150 | FastLED + PDM mic | Audio reactive, scene color palette |

### Pixelblaze V3 Standard (independent)

| Zone | LEDs | Notes |
|---|---|---|
| Rear window | 150 | Cut candidate — Pixelblaze now has no primary zone; deprioritise if time is short |

Pixelblaze is configured independently via its web UI. It does not receive serial or ESP-NOW commands for MVP. Scene sync to Pixelblaze is a future enhancement.

---

## Power Distribution

### Car 12V (ignition-switched)

- Arduino Uno R4 WiFi
- WH1602B-TMI-JT 16×2 character LCD (5V from Uno R4)
- GC9A01 round TFT IRIS display (via ESP32-S3, 3.3V)
- Pioneer head unit (existing)
- X9C104 digital pot (existing)
- Offroad lights/lightbars via 12V relay (T1 toggle, MCU-independent)

### Battery USB-C 100W (exterior module)

- USB-C PD trigger board (negotiates 5V, min 15A rated)
- ESP32-S3 exterior node
- All exterior WS2812B strips via 5V rail
- Wheel well static strips via MOSFET PWM
- Pixelblaze V3 Standard (shares 5V rail or dedicated tap)

### Battery USB-C 18W (interior module)

- USB-C PD trigger board (negotiates 5V)
- ESP32-S3 N16R8 (interior hub)
- Dashboard WS2812B strips via 5V rail
- Floor footwell static strips via MOSFET PWM

### Car 12V — IRIS display module (permanent)

- Hosyond ESP32-S3 N16R8 (IRIS display character)
- 1.28" round GC9A01 TFT (240×240)
- Powered from car 12V via 3.3V regulator (display and ESP32 are 3.3V logic)

Note: USB-C PD trigger boards required for lighting modules. Cannot wire LED strips directly to USB-C. Purchase boards rated for target amperage before wiring.

---

## Code

### arduino/PioneerController/
- `PioneerController.ino`: merged sketch — Pioneer control, serial event emission, protothread3 for buttons/toggle
- `X9C.h` / `X9C.cpp`: digital pot driver (MIT, Phil Bowles 2017)
- `pt.h`, `lc.h`, `lc-switch.h`, `lc-addrlabels.h`, `pt-sem.h`: Protothreads (Contiki OS, Adam Dunkels)

### esp32/interior-hub/
- `interior-hub.ino`: ESP32-S3 sketch — serial listener, glitter animation, scene management, exterior UART TX (port from RP2040 complete)

### esp32/exterior-node/
- `exterior-node.ino`: ESP32-S3 N16R8 sketch — serial listener, glitter animation across side/hood/accent zones

### esp32/sensor-bar/
- Firmware not yet written — XIAO ESP32-S3 Sense, PDM mic + FastLED, scene-aware audio reactivity. See `README.md`.

### display-code/IrisDisplay/
- `IrisDisplay.ino`: IRIS display sketch — GC9A01 round TFT on ESP32-S3, TFT_eSPI library
- `User_Setup.h`: TFT_eSPI pin config for GC9A01 (update GPIOs on hardware receipt)
- `iris-display-spec.md`: Full phase roadmap and visual identity spec

### arduino/character-lcd-spec.md
- WH1602B-TMI-JT 16×2 LCD spec — pin assignments, state machine, content, wiring notes

### Integration Approach

Preserve existing Pioneer/X9C/Protothreads code with minimal changes. Add:
- Pin constant definitions at top of file
- Toggle T2 read on boot and interrupt/poll
- Serial event emission on each input event
- Scene suppression logic when T2 is OFF

Do not refactor Protothreads to FreeRTOS at this stage. That migration is planned post-event when the head unit code moves to ESP32-S3 for permanent install.

---

## Coding Standards

- All pin numbers as named constants, no magic numbers anywhere
- All event strings as named constants (e.g. `const char* EVT_VOL_UP = "VOL_UP\n"`)
- Comments on non-obvious logic, especially timing-sensitive sections
- No blocking delays in main loop or lighting animation
- Each ESP32 file clearly labeled with its role at the top

---

## Deferred / Out of Scope for MVP

- Foglight BT integration: run proprietary app for the event, revisit post-event
- Reactive audio lighting: Pixelblaze stretch goal, configure via Pixelblaze web UI independently
- Pixelblaze scene sync: future enhancement
- OBD-II / CAN bus integration: future car project
- ESP32 camera surveillance feed: hardware available, implementation deferred
- Wheel well static strips: stretch goal, may run standalone
- Rear window strip: optional, cut candidate under time pressure
- FreeRTOS migration: post-event with ESP32-S3 permanent install

---

## Open Hardware Tasks (Pre-Implementation)

- [ ] Purchase USB-C PD trigger boards (100W and 18W rated)
- [ ] Confirm MOSFET selection for static RGB strips (IRLZ44N or equivalent 3.3V gate drive compatible)
- [ ] Measure and plan wire runs for power injection points on long LED strips
- [ ] Confirm Pixelblaze V3 Standard power input spec and tap from exterior 5V rail
- [ ] Source Cherry MX buttons and keycaps for console panel
- [ ] 3D print center console mount
