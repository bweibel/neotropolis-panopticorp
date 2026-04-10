# Baja Lighting Controller: Implementation Spec

## Purpose

This document is a handoff spec for Claude Code. It defines the full software architecture for the Baja AV and lighting controller system. Hardware selection and architecture decisions are finalized. Implementation should follow this spec without revisiting those decisions.

---

## System Overview

Two independent software systems sharing a one-way serial link.

**Permanent system** (Arduino Uno R4 WiFi, car 12V ignition-switched):
- Reads physical inputs (encoder, buttons, toggles)
- Controls Pioneer head unit via X9C104 digital pot
- Broadcasts serial events regardless of whether anything is listening

**Removable lighting module** (battery powered):
- QT Py RP2040 Interior Hub: listens to Uno serial, controls interior strips, sends scene index to exterior node via wired UART
- ESP32-S3 Exterior Node: receives scene index via wired UART from hub, controls exterior strips and Pixelblaze

---

## Hardware Map

| Device | Role | Connection |
|---|---|---|
| Arduino Uno R4 WiFi | Head unit controller, input surface | Car 12V |
| QT Py RP2040 (Adafruit 4900) | Interior lighting hub | Battery 18W USB-C |
| ESP32-S3 (with camera) | Exterior lighting node | Battery 100W USB-C |
| Pixelblaze V3 Standard | Top scanner + rear window strips | Battery (via exterior node power rail) |
| LCD display | Mode and status display | Uno R4 |

---

## Physical Inputs (Uno R4)

### Toggle Switches (between seats, seat heater location)

| Toggle | Function | MCU involvement |
|---|---|---|
| T1 | Offroad lights / lightbars (12V relay) | None, fully independent |
| T2 | LED master power (cuts 12V to lighting module) | Uno reads state on boot |

Toggle T2 is read by the Uno on boot and on change. When OFF, Uno suppresses SCENE_* events. All other events (VOL, SKIP, MUTE) are always emitted.

### Center Console Panel (3D printed mount, Uno R4 visible)

| Input | Pin (assign as constant) | Function |
|---|---|---|
| Rotary encoder CLK | ENC_CLK | Volume up/down |
| Rotary encoder DT | ENC_DT | Volume direction |
| Rotary encoder SW | ENC_SW | Mute toggle |
| Button 1 | BTN_1 | Skip forward |
| Button 2 | BTN_2 | Skip back |
| Button 3 | BTN_3 | Scene cycle |
| Button 4 (optional) | BTN_4 | Reserved / future use |
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

## Interior→Exterior Link

- Transport: Wired UART, one-way TX from RP2040 hub to ESP32 exterior node
- Baud: 9600
- Format: ASCII integer, newline terminated ("0\n", "1\n", "2\n")
- Content: Scene index only
- No handshake, no acknowledgment

---

## Lighting Zones and Controllers

### Interior Hub (ESP32 #1)

| Zone | Type | LEDs | Control method | Notes |
|---|---|---|---|---|
| Dashboard outline x2 | WS2812B 2.7mm | 160 each | FastLED, two data pins | Potential permanent install |
| Floor footwells x2-4 | Static 4-line RGB | unknown | MOSFET PWM (3 pins per zone) | N-ch MOSFETs e.g. IRLZ44N |

### Exterior Node (ESP32 #2)

| Zone | Type | LEDs | Control method | Notes |
|---|---|---|---|---|
| Side strips x2 | WS2812B | 150 each | FastLED, two data pins | Set and forget, basic animation |
| Hood scoop | WS2812B | <10 | FastLED, join nearest data line | Static acceptable |
| Accent elements | WS2812B | ~8 each | FastLED, join nearest data line | Basic animation |
| Wheel wells x4 | Static 4-line RGB | unknown | MOSFET PWM (stretch goal) | Can run standalone if cut |

### Pixelblaze V3 Standard (independent)

| Zone | LEDs | Notes |
|---|---|---|
| Top scanner | 150 | Primary reactive zone, audio reactive stretch goal |
| Rear window | 150 | Optional, cut candidate under time pressure |

Pixelblaze is configured independently via its web UI. It does not receive serial or ESP-NOW commands for MVP. Scene sync to Pixelblaze is a future enhancement.

---

## Power Distribution

### Car 12V (ignition-switched)

- Arduino Uno R4 WiFi
- LCD display
- Pioneer head unit (existing)
- X9C104 digital pot (existing)
- Offroad lights/lightbars via 12V relay (T1 toggle, MCU-independent)

### Battery USB-C 100W (exterior module)

- USB-C PD trigger board (negotiates 5V, min 15A rated)
- ESP32 #2 (exterior node)
- All exterior WS2812B strips via 5V rail
- Wheel well static strips via MOSFET PWM from ESP32
- Pixelblaze V3 Standard (shares 5V rail or dedicated tap)

### Battery USB-C 18W (interior module)

- USB-C PD trigger board (negotiates 5V)
- ESP32 #1 (interior hub)
- Dashboard WS2812B strips via 5V rail
- Floor footwell static strips via MOSFET PWM from ESP32

Note: USB-C PD trigger boards required. Cannot wire LED strips directly to USB-C. Purchase boards rated for target amperage before wiring.

---

## Existing Code

### Files

- `PioneerController-ProtoThread.ino` (~420 lines): main sketch
- `X9C.h` / `X9C.cpp`: digital pot driver (MIT, Phil Bowles 2017)
- `pt.h`, `lc.h`, `lc-switch.h`, `lc-addrlabels.h`, `pt-sem.h`: Protothreads (Contiki OS, Adam Dunkels)

### Known Bugs to Fix

Both are off-by-one array boundary errors:

1. `IncreaseQueueIndex()`: checks `> QUEUEMAXSIZE` should be `>= QUEUEMAXSIZE`
2. `ClearQueue()`: iterates `<= QUEUEMAXSIZE` should be `< QUEUEMAXSIZE`

Both write one element past the end of `QueueCommands[QUEUEMAXSIZE]`. Fix before adding new functionality.

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
