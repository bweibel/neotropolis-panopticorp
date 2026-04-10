# Agent Handoff: Fix Off-by-One Bugs in PioneerController Queue

## What this is

This is a task handoff for Claude Code. You are working in the repo at the current working directory on branch `vehicle/lighting-controller`.

---

## Context

`vehicle/electronics/arduino/PioneerController/PioneerController.ino` is an Arduino sketch for an Arduino Uno R4 WiFi. It controls a Pioneer car head unit via an X9C104 digital potentiometer and uses a Protothreads-based command queue system.

There are **two known off-by-one array boundary bugs** in the queue functions. Both write one element past the end of `QueueCommands[QUEUEMAXSIZE]`. These must be fixed before any new functionality is added.

---

## The Bugs

The bugs are described precisely in the spec at `vehicle/electronics/baja-lighting-spec.md` (lines 205–210):

> 1. `IncreaseQueueIndex()`: checks `> QUEUEMAXSIZE` should be `>= QUEUEMAXSIZE`
> 2. `ClearQueue()`: iterates `<= QUEUEMAXSIZE` should be `< QUEUEMAXSIZE`

Both write one element past the end of `QueueCommands[QUEUEMAXSIZE]`.

---

## Your Task

1. Read `PioneerController.ino` in full.
2. Find `IncreaseQueueIndex()` — change the boundary check from `> QUEUEMAXSIZE` to `>= QUEUEMAXSIZE`.
3. Find `ClearQueue()` — change the loop condition from `<= QUEUEMAXSIZE` to `< QUEUEMAXSIZE`.
4. Make no other changes. Do not refactor, rename, or reformat anything else.
5. Remove the bug-fix TODO comment (lines referencing these two bugs) from the sketch once the fixes are applied.

---

## Constraints

- **Do not** touch any other logic — Protothreads, X9C driver, Pioneer command table, setup(), loop().
- **Do not** add new features or serial event emission code. That comes after these fixes.
- The Protothreads and X9C library files (`pt.h`, `X9C.h`, etc.) may or may not be present in the directory — leave them as-is either way.
- This is embedded C++ (Arduino). Standard Arduino/C++ conventions apply.

---

## Verification

After making changes:
- Confirm the array `QueueCommands` is declared as `QueueCommands[QUEUEMAXSIZE]` (size N).
- `IncreaseQueueIndex()` should now guard against index == QUEUEMAXSIZE (i.e., wrap or clamp before going out of bounds).
- `ClearQueue()` should now iterate indices 0 through QUEUEMAXSIZE-1 inclusive.
- `git diff` should show exactly two one-character changes (the two boundary operators).
