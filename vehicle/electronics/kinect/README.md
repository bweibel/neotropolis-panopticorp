# Panopticorp Surveillance Terminal — Kinect Display

**Role:** Standalone interactive surveillance exhibit. Xbox 360 Kinect mounted externally on the Baja detects and tracks subjects; Raspberry Pi 4 (cyberdeck) processes sensor data and drives a small HDMI screen in the rear window showing a Panopticorp-themed surveillance feed. Fully independent of the lighting system.

## Status: SHELVED — stretch goal. Kinect v1 requires proprietary connector rewiring before USB connection is possible. Deprioritised for May 2026 event.

## Hardware

| Device | Role | Notes |
|---|---|---|
| Xbox 360 Kinect (v1) | Depth + RGB sensor, 4-mic array | USB 2.0, needs external 12V |
| Raspberry Pi 4 | Processing and display host | Existing cyberdeck unit |
| Small HDMI/USB-C screen | Rear window display | Existing, on hand |

### Power
- Kinect v1: 12V from car ignition rail + USB data to Pi 4
- Pi 4: existing cyberdeck power supply
- Screen: USB-C from Pi 4 or dedicated supply

### Physical mounting
- Kinect: exterior rear mount (roof edge or rear bumper) facing crowd
- USB cable routed inside to Pi
- Screen: rear window, visible from outside
- Weatherproofing required on Kinect connector joint (outdoor event)

## Software Stack

- **OS:** Raspberry Pi OS (64-bit)
- **Kinect driver:** `libfreenect` with Python bindings (`freenect`)
- **Vision / rendering:** OpenCV (`cv2`)
- **Display:** OpenCV fullscreen window to HDMI output
- **Gesture detection:** Depth blob analysis (no NITE/skeleton lib required)

## Interaction Model

| State | Trigger | Display |
|---|---|---|
| IDLE | No subject in range (>2.5m) | Panopticorp logo, animated scan lines, "SURVEILLANCE ACTIVE" |
| DETECTING | Subject enters range (~2.5m) | Depth map activates, "SUBJECT DETECTED", scan progress bar begins |
| SCANNING | Subject holds position ~1s | Skeleton overlay populates, fake biometrics animating |
| IDENTIFIED | Scan complete | Fabricated dossier — subject ID, threat level, fake vitals readout |
| INTERACTION | Raised hand or wave gesture | Acknowledged response animation, easter egg potential |

State transitions are driven by:
- **Proximity zones:** depth frame nearest-blob distance thresholding
- **Dwell time:** subject must hold position to advance DETECTING → SCANNING
- **Gesture detection:** highest point of nearest blob significantly above centroid = raised hand; horizontal centroid oscillation = wave

## Display Aesthetic

- RGB camera feed as background (people see themselves — primary engagement hook)
- Dark semi-transparent overlay
- Panopticorp HUD elements: corner brackets, scan line sweep, fabricated data panels
- Text: `PANOPTICORP SURVEILLANCE DIVISION`, subject ID (random hash), threat level, fake vitals
- Color palette: match vehicle scene colors (red/green) or amber-on-black terminal aesthetic

## Phases

### Phase 1 — Kinect Functionality Test (START HERE)
Get libfreenect running on Pi 4 and verify sensor output.

1. Install libfreenect: `sudo apt install freenect`
2. Install Python bindings: `pip install freenect` (or build from source if apt version is stale)
3. Connect Kinect via USB to Pi 4, power via 12V supply
4. Run depth stream test — print frame shape and min/max depth values to confirm data flowing
5. Run RGB stream test — display live camera feed in OpenCV window
6. Confirm 30fps depth + RGB simultaneously
7. Verify mic array is detected as audio input device

Deliverable: short test script `kinect_test.py` that opens both streams, displays them side by side, prints fps to console. Confirms hardware is functional before any UI work.

### Phase 2 — Proximity Detection and State Machine
8. Threshold depth frame to isolate nearest blob (subject)
9. Compute blob distance (median depth of nearest region)
10. Implement IDLE / DETECTING / SCANNING / IDENTIFIED state transitions
11. Print state transitions to console (no UI yet)

### Phase 3 — Gesture Detection
12. Track blob centroid and highest point per frame
13. Detect raised hand: highest point > centroid_y - threshold
14. Detect wave: centroid_x oscillation > threshold over N frames
15. Emit gesture events into state machine

### Phase 4 — Display UI
16. Implement each state's display composition in OpenCV
17. Fabricated dossier content (random subject ID, static fake vitals)
18. HUD overlay elements: corner brackets, scan lines, progress bar
19. RGB camera feed as background
20. Output fullscreen to HDMI screen

### Phase 5 — Polish and Integration
21. Tune proximity thresholds against actual outdoor conditions
22. Weatherproof Kinect connector for exterior mount
23. Cable management: USB + 12V run from exterior mount to Pi inside
24. Test at event distance (~2–3m typical crowd standoff)

## Open Items

- [ ] Confirm Kinect v1 USB adapter cable on hand (splits proprietary connector → USB-A + barrel power)
- [ ] Confirm 12V tap point on car rail for Kinect power
- [ ] Decide display color palette: scene red/green vs. amber terminal aesthetic
- [ ] Decide fabricated dossier content (flavor text, fake org names, threat categories)
- [ ] Confirm screen mounting method in rear window

## Handoff Notes

- Pi 4 is an existing cyberdeck unit — assume Raspberry Pi OS 64-bit, Python 3, standard apt packages available
- libfreenect Python bindings may need to be built from source if pip package is outdated; see [libfreenect GitHub](https://github.com/OpenKinect/libfreenect) for build instructions
- Kinect v1 depth range: ~0.5m–4.5m reliable; outdoor ambient IR can reduce effective range — test in real conditions
- No connection to the Uno/ESP32 lighting system required. This is fully standalone.
- See [baja-lighting-spec.md](../baja-lighting-spec.md) for vehicle context
