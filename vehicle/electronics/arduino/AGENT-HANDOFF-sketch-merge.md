# Agent Handoff: Merge PioneerController sketch files

## What this is

This is a task handoff for Claude Code. You are working in the repo at the current working directory on branch `vehicle/lighting-controller`.

---

## The Problem

`vehicle/electronics/arduino/PioneerController/` contains two `.ino` files. Arduino compiles all `.ino` files in a sketch folder together, so having `setup()` and `loop()` defined in both causes a duplicate function error at build time.

| File | Contains |
|---|---|
| `PioneerController.ino` | Pin constants, event string constants, SoftwareSerial setup, `ledMasterOn` state, stub `setup()` and `loop()` with TODOs |
| `PioneerController-ProtoThread.ino` | Full implementation: X9C pot, Protothreads queue, encoder reading, Pioneer command dispatch, `setup()`, `loop()`, `IncreaseQueueIndex()`, `ClearQueue()` |

The goal is a single `PioneerController.ino` containing everything, with `PioneerController-ProtoThread.ino` deleted.

---

## What to keep from each file

### From `PioneerController.ino` — keep and integrate:
- The role/purpose comment block at the top
- `#include <SoftwareSerial.h>`
- All named pin constants (`ENC_CLK`, `ENC_DT`, `ENC_SW`, `BTN_1`–`BTN_4`, `TOGGLE_LED`, `SERIAL_TX`, `SERIAL_RX`) — these replace the old `#define clkPin`, `#define dtPin`, `#define swPin` magic-number defines
- All event string constants (`EVT_VOL_UP`, `EVT_VOL_DOWN`, `EVT_MUTE`, `EVT_SKIP_FWD`, `EVT_SKIP_BACK`, `EVT_SCENE_NEXT`)
- `SoftwareSerial espSerial(SERIAL_RX, SERIAL_TX);`
- `bool ledMasterOn = false;`
- The TODO stubs in `setup()` and `loop()` for serial event emission and input reading — fold these into the real implementation described below

### From `PioneerController-ProtoThread.ino` — keep everything except:
- The old `#define clkPin 2`, `#define dtPin 3`, `#define swPin 4` pin defines — replace with the named constants from the new file
- The old `#define CS 13`, `#define UD 12`, `#define INC 11` pot pin defines — replace with named constants (add `PIN_POT_CS`, `PIN_POT_UD`, `PIN_POT_INC` to the constants block with the same values)
- The `Serial.begin(115200)` in `setup()` — remove or keep for debug, but note the new file opens `espSerial` at 9600 which must be preserved

---

## Merge approach

1. Start with the header/includes block from `PioneerController.ino`.
2. Add `#include "X9C.h"`, `#include "pt.h"` (uncommented).
3. Add the pin constants block, expanded to include pot pins:
   ```cpp
   const int PIN_POT_CS  = 13;
   const int PIN_POT_UD  = 12;
   const int PIN_POT_INC = 11;
   ```
4. Add event string constants block.
5. Add `SoftwareSerial espSerial(...)` and `bool ledMasterOn`.
6. Add all globals from `PioneerController-ProtoThread.ino` (encoder vars, time vars, Protothreads structs, queue, command defines, pot object) — unchanged except replacing `clkPin`/`dtPin`/`swPin`/`CS`/`UD`/`INC` references with the named constants.
7. Merge `setup()`:
   - Keep pot init, `ClearQueue()`, from the old file
   - Keep `espSerial.begin(9600)`, `pinMode` calls, `ledMasterOn` read from the new file
   - Keep `Serial.begin(115200)` for debug
   - Add `// TODO: initialize LCD`
8. Merge `loop()`:
   - Keep `protothread1(&pt1)` and `protothread2(&pt2)` calls
   - Add TODOs for serial event emission and `TOGGLE_LED` polling (these are not yet implemented — leave as comments)
9. Keep all other functions unchanged (`protothread1`, `protothread2`, `PulseVolumeUp`, etc., `getEncoderTurn`, `IncreaseQueueIndex`, `ClearQueue`).
10. Delete `PioneerController-ProtoThread.ino`.

---

## Constraints

- Do not refactor Protothreads logic, queue logic, or Pioneer command dispatch. Preserve as-is.
- Do not add the serial event emission implementation — that is future work. Leave the TODOs.
- No magic numbers: all pin references must use the named constants after merge.
- The `IncreaseQueueIndex()` and `ClearQueue()` bugs have already been fixed in the existing file — do not re-introduce the old versions.

---

## Verification

After merge:
- Only one `.ino` file exists in the directory (`PioneerController.ino`).
- `PioneerController-ProtoThread.ino` is deleted.
- No duplicate `setup()` or `loop()` definitions.
- No `#define clkPin`, `#define dtPin`, `#define swPin`, `#define CS`, `#define UD`, `#define INC` — all replaced by `const int` named constants.
- `IncreaseQueueIndex()` still uses `>= QUEUEMAXSIZE` (not `>`).
- `ClearQueue()` loop still uses `< QUEUEMAXSIZE` (not `<=`).
- `espSerial.begin(9600)` is present in `setup()`.
- `protothread1` and `protothread2` are called in `loop()`.
