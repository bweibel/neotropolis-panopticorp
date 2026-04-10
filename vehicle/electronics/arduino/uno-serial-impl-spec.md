# Uno R4 Serial Event Emission: Implementation Spec

## Purpose

This document specifies how to add serial event emission to `PioneerController.ino`. The existing Protothreads/queue/X9C logic is complete and must not be touched. This spec covers only the new work.

---

## What needs to happen

Six serial events must be emitted over `espSerial`. Five are triggered by existing code paths (encoder and encoder button). Three need new button reading logic (BTN_1, BTN_2, BTN_3). TOGGLE_LED needs polling to maintain `ledMasterOn`.

**Button intent:**
- BTN_1 / BTN_2: Primary purpose is Pioneer track control. Serial events also emitted for future LCD response.
- BTN_3: Primary purpose is lighting scene cycle. Serial event emitted; no Pioneer command. Future LCD response possible.

---

## Part 1: Serial emission on existing code paths

The `Pulse*()` helper functions are the right place. Each already queues a Pioneer command and prints to `Serial`. Add `espSerial.print(EVT_*)` to each:

| Function | Add after queue push |
|---|---|
| `PulseVolumeUp()` | `espSerial.print(EVT_VOL_UP)` |
| `PulseVolumeDown()` | `espSerial.print(EVT_VOL_DOWN)` |
| `PulseMute()` | `espSerial.print(EVT_MUTE)` |
| `PulseTrackForward()` | `espSerial.print(EVT_SKIP_FWD)` |
| `PulseTrackBack()` | `espSerial.print(EVT_SKIP_BACK)` |

Note: `PulseTripleClick()` has no corresponding serial event — leave it alone.

These five functions are called from `protothread1`. No changes to protothread logic needed.

---

## Part 2: New button and toggle reading (protothread3)

BTN_1, BTN_2, BTN_3, and TOGGLE_LED are not read anywhere in the current code. Add a third protothread to handle them — consistent with the existing pattern.

### Add to globals:

```cpp
static struct pt pt3;
int lastToggleState = HIGH;  // initialized in setup() from actual pin state — see Part 3
```

### Add to `loop()`, after the existing two protothread calls:

```cpp
protothread3(&pt3);
```

### New function — `protothread3`:

```cpp
static int protothread3(struct pt *pt)
{
  static unsigned long timestamp = 0;
  static int lastBtn1 = HIGH;
  static int lastBtn2 = HIGH;
  static int lastBtn3 = HIGH;
  // lastToggleState is a global set in setup() from the actual pin — avoids spurious change on first loop
  // (a static local can't be initialized from a runtime value)

  PT_BEGIN(pt);
  while (1)
  {
    int btn1 = digitalRead(BTN_1);
    int btn2 = digitalRead(BTN_2);
    int btn3 = digitalRead(BTN_3);
    int tog  = digitalRead(TOGGLE_LED);
    int& lastToggle = lastToggleState;  // alias to global set in setup()

    // BTN_1: Skip forward
    if (btn1 == LOW && lastBtn1 == HIGH) {
      PulseTrackForward();  // queues Pioneer command + emits EVT_SKIP_FWD (Part 1)
    }
    lastBtn1 = btn1;

    // BTN_2: Skip back
    if (btn2 == LOW && lastBtn2 == HIGH) {
      PulseTrackBack();     // queues Pioneer command + emits EVT_SKIP_BACK (Part 1)
    }
    lastBtn2 = btn2;

    // BTN_3: Scene cycle — serial only, no Pioneer command, suppressed when LED master off
    if (btn3 == LOW && lastBtn3 == HIGH) {
      if (ledMasterOn) {
        espSerial.print(EVT_SCENE_NEXT);
        Serial.println("SCENE_NEXT");
      }
    }
    lastBtn3 = btn3;

    // TOGGLE_LED: update ledMasterOn on change
    if (tog != lastToggle) {
      ledMasterOn = (tog == LOW);
      Serial.print("LED master: ");
      Serial.println(ledMasterOn ? "ON" : "OFF");
    }
    lastToggle = tog;

    timestamp = millis(); PT_WAIT_UNTIL(pt, millis() - timestamp > DeBounceDelay);
  }
  PT_END(pt);
}
```

### Notes on this approach:

- `DeBounceDelay` (10ms) is reused — already proven adequate for this hardware.
- BTN_1/BTN_2 call `PulseTrackForward()` / `PulseTrackBack()`, which queue the Pioneer command AND (after Part 1) emit serial events. One call does both — no duplication.
- BTN_1/BTN_2 call `PulseTrackForward()` / `PulseTrackBack()`, which queue the Pioneer command AND emit serial events. One call does both — no duplication. LCD can respond to the same serial events downstream.
- BTN_3 emits serial only. There is no Pioneer command for scene cycle. LCD response can be added downstream without changing this code.
- The encoder SW (ENC_SW) single/double/hold logic in `protothread1` remains unchanged — it independently controls skip and mute via the encoder button. BTN_1/BTN_2 are separate physical buttons that do the same thing but are easier to reach.
- TOGGLE_LED: `ledMasterOn` is updated on change. No serial event emitted — revisit if needed.

**TODO (LCD implementation phase):** BTN_3 `SCENE_NEXT` emission is currently gated on `ledMasterOn`. If the LCD needs to respond to BTN_3 regardless of LED master state, move the suppression logic downstream (hub or LCD handler) rather than here. Revisit when LCD items are being implemented.

---

## Part 3: lastToggleState initialization

`lastToggleState` must be set in `setup()` after `pinMode(TOGGLE_LED, INPUT_PULLUP)`, so the first loop iteration sees no spurious change:

```cpp
lastToggleState = digitalRead(TOGGLE_LED);
```

Place this immediately after the `ledMasterOn` assignment (they read the same pin and should stay together).

---

## Part 4: PT_INIT

`pt1` and `pt2` are not explicitly initialized in `setup()` — the existing code relies on zero-init of static structs. Use the same pattern for `pt3`: the `static struct pt pt3` declaration is sufficient, no `PT_INIT` call needed.

For clarity, add an explicit `PT_INIT` for all three in `setup()`. Add alongside `espSerial.begin()`:

```cpp
PT_INIT(&pt1);
PT_INIT(&pt2);
PT_INIT(&pt3);
```

This makes initialization intent unambiguous without changing behavior.

---

## Summary of changes

| Location | Change |
|---|---|
| `PulseVolumeUp()` | Add `espSerial.print(EVT_VOL_UP)` |
| `PulseVolumeDown()` | Add `espSerial.print(EVT_VOL_DOWN)` |
| `PulseMute()` | Add `espSerial.print(EVT_MUTE)` |
| `PulseTrackForward()` | Add `espSerial.print(EVT_SKIP_FWD)` |
| `PulseTrackBack()` | Add `espSerial.print(EVT_SKIP_BACK)` |
| Globals | Add `static struct pt pt3` |
| `loop()` | Add `protothread3(&pt3)`, remove TODO comments |
| New function | `protothread3` (BTN_1, BTN_2, BTN_3, TOGGLE_LED) |
| `setup()` | Add `PT_INIT(&pt1/2/3)`, set `lastToggleState = digitalRead(TOGGLE_LED)` |

No changes to `protothread1`, `protothread2`, `setup()` internals, or queue logic.

---

## Verification

- Turning encoder clockwise: Pioneer volume goes up AND `espSerial` emits `VOL_UP\n`
- Turning encoder counter-clockwise: Pioneer volume goes down AND `espSerial` emits `VOL_DOWN\n`
- Long-hold encoder button: Pioneer mutes AND `espSerial` emits `MUTE\n`
- Single/double-click encoder button: track forward/back AND `espSerial` emits `SKIP_FWD\n` / `SKIP_BACK\n`
- BTN_1 press: same as track forward — Pioneer queued AND `espSerial` emits `SKIP_FWD\n`
- BTN_2 press: same as track back — Pioneer queued AND `espSerial` emits `SKIP_BACK\n`
- BTN_3 press with `ledMasterOn = true`: `espSerial` emits `SCENE_NEXT\n`
- BTN_3 press with `ledMasterOn = false`: nothing emitted
- Flipping TOGGLE_LED: `ledMasterOn` updates, no serial event
- Monitor `espSerial` TX line at 9600 baud to confirm strings; use `Serial` debug output for lab testing
