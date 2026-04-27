# IRIS Display: Implementation Spec

## Scope

Drive a GC9A01 1.28" round TFT (240×240) on a dedicated Seeed XIAO ESP32-S3 Sense. Display the IRIS surveillance AI character — an animated eye lens filling the circular canvas. No housing chrome, no status bar, no ticker zone; the round format IS the eye.

---

## Hardware

- **Display:** 1.28" round TFT, GC9A01 controller, 240×240, SPI, 3.3V, IPS panel
- **MCU:** Hosyond ESP32-S3 N16R8 (16MB flash, 8MB OPI PSRAM)
- **Power:** Car 12V permanent (same rail as Uno R4)
- **Library:** GFX Library for Arduino (Arduino_GFX, moononournation) v1.5.6+
- **Toolchain:** `arduino-cli`
- **Board target:** `esp32:esp32:esp32s3:PSRAM=opi,CDCOnBoot=cdc`

### Library notes

TFT_eSPI (Bodmer) crashes on this hardware with a StoreProhibited panic inside `tft.init()` — a known DMA/PSRAM compatibility issue on ESP32-S3. Arduino_GFX works correctly.

The GC9A01 modules used are IPS panels — `ips=true` must be passed to the `Arduino_GC9A01` constructor or colors are inverted.

`PSRAM=opi` is required in the FQBN. Without it, TFT_eSPI and Arduino_GFX both crash on boot with a null-pointer dereference in DMA setup.

### Pin assignment — Hosyond ESP32-S3 N16R8

GPIOs 26–37 reserved (flash + OPI PSRAM). All assignments below are outside that range.

| Signal     | GPIO | Notes                                              |
|------------|------|----------------------------------------------------|
| MOSI (SDA) | 11   | Hardware SPI2 MOSI                                 |
| SCK (SCL)  | 12   | Hardware SPI2 clock                                |
| CS         | 10   |                                                    |
| DC         | 9    |                                                    |
| RST        | 8    |                                                    |
| VCC        | 3.3V | 3.3V logic — no onboard regulator on these modules |
| GND        | GND  |                                                    |

Serial RX (from Interior Hub): GPIO 17.

---

## Arduino_GFX initialisation

```cpp
Arduino_ESP32SPI bus(9, 10, 12, 11, GFX_NOT_DEFINED);  // DC, CS, SCK, MOSI, MISO
Arduino_GC9A01   display(&bus, 8, 0, true);              // RST, rotation, IPS=true
Arduino_Canvas   canvas(240, 240, &display);
```

---

## Serial input

Receives scene index from Interior Hub via wired UART (shared Serial2 broadcast TX).

- Baud: 9600
- Format: ASCII integer + newline (`"0\n"`, `"1\n"`, `"2\n"`)
- Same format as exterior node — both wired to the same TX pin on the hub

```cpp
// GPIO 17 on devkit, GPIO 44 on XIAO
Serial1.begin(9600, SERIAL_8N1, PIN_IRIS_RX, -1);  // RX only
```

---

## Phase roadmap

| Phase | Scope | Status |
|---|---|---|
| 1 | Wiring test — confirms GC9A01 initialises and renders | **Complete** (devkit) |
| 2 | Static IRIS eye — cream oval, almond slit, pupil dot, all mood draw functions stubbed | **Complete** (devkit) |
| 3 | Animated mood states — interpolate eyelid planes, pupil size/offset; full glitch scanlines | Not started |
| 4 | Scene-driven color via UART — parses scene index from Interior Hub | Not started |
| 5 | Mood state machine — mood transitions driven by scene + autonomous timing | Not started |

---

## Compile and flash

```sh
arduino-cli compile --fqbn esp32:esp32:esp32s3:PSRAM=opi,CDCOnBoot=cdc IrisDisplay/
arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:esp32s3:PSRAM=opi,CDCOnBoot=cdc IrisDisplay/
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```

---

## Phase 2: IRIS eye character

See mockups: `assets/brand/iris/iris-mock.png`, `assets/brand/iris/iris-moods.png`

The eye fills the circular 240×240 canvas. All expression is in the eye — no housing chrome, no status bar.

### Rendering layers (draw order)

1. **Black background** — `fillScreen(COLOR_BLACK)`
2. **Cream oval sclera** — `fillEllipse(cx, cy, OVAL_RX, OVAL_RY, COLOR_CREAM)` — horizontal oval, warm off-white
3. **Almond slit** — `fillLens()` scanline function, two-circle intersection → pointed ends; cut into sclera in black
4. **Eyelid planes** — black `fillRect` from top and/or bottom of oval; clipping amount encodes mood
5. **Pupil dot** — `fillCircle` in white (green in glitch state)

### Pixel grid

All geometry is rendered on a logical pixel grid. `PIXEL=4` maps each logical pixel to a 4×4 block on the 240×240 display (60×60 logical grid).

| Constant | Value | Description |
|---|---|---|
| `PIXEL` | 4 | Physical pixels per logical pixel |
| `PCX/PCY` | 30 | Logical center (120/PIXEL) |

### Eye geometry constants

| Symbol | Value | Description |
|---|---|---|
| `OVAL_RX` | 26 | Sclera semi-width (logical px) |
| `OVAL_RY` | 17 | Sclera semi-height (logical px) |
| `COLOR_CREAM` | `color565(238, 230, 196)` | Warm off-white sclera — set post-init |
| `COLOR_PUPIL` | `COLOR_CREAM` | Pupil dot — matches sclera |

Note: `pFillEllipse` and `pFillCircle` skip rows where `hw==0` to avoid lone tip pixels. One stray pixel remains at left/right of the slit at the widest row — acceptable at this resolution.

### Mood states

All six moods are stubbed as draw functions in Phase 2. Phase 3 animates between them.

| Mood | topLidY | botLidY | slitRx | slitRy | pR | pColor | Read |
|---|---|---|---|---|---|---|---|
| Neutral | −OVAL_RY | +OVAL_RY | 16 | 7 | 3 | cream | Open, passive |
| Squint | −6 | +6 | 76 | 20 | 10 | white | Nearly closed, calculating |
| Surprised | −OVAL_RY | +OVAL_RY | 76 | 28 | 18 | white | Wider slit, larger dot |
| Suspicious | −10 | +32 | 76 | 20 | 11 | white | Top eyelid heavy, offset pupil |
| Wide scan | −OVAL_RY | +OVAL_RY | 88 | 24 | 15 | white | Wider slit, active sweep |
| Glitch | −OVAL_RY | +OVAL_RY | 76 | 20 | 16 | **green** | Full scanlines deferred to Phase 3 |

`topLidY` and `botLidY` are y offsets from `LCD_CY`. A top eyelid covers from `LCD_CY − OVAL_RY` down to `LCD_CY + topLidY`.

### Color modes (Phase 4)

`scleraColor` is passed into `drawIris()` — change it per scene index:

- **Scene 0 (OFF):** `COLOR_CREAM` — default, corporate/friendly
- **Scene 1 (RED):** tinted red sclera — surveillance/patrol
- **Scene 2 (GREEN):** tinted green sclera — sweep mode

Glitch state always overrides pupil to `COLOR_GLITCH_GREEN` regardless of scene.

---

## Open items

- [ ] Tune OVAL_RX, OVAL_RY, slitRx, slitRy on hardware against mockup (Phase 2 polish)
- [ ] Confirm `setRotation` value when physically mounted in housing
- [ ] Confirm `setRotation()` value when physically mounted in housing
- [ ] Confirm camera pin conflicts on XIAO — if OV2640 in use, verify CS/DC/RST (GPIOs 2/3/4)
- [ ] Scene → mood mapping decision (Phase 5)
- [ ] Glitch trigger frequency and duration (Phase 5)
- [ ] Glitch scanline distortion implementation (Phase 3)
