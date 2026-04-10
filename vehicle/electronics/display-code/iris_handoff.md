# IRIS Display Character — Handoff Summary

**Project:** Panopticorp Remote Surveillance Unit / Neotropolis 2026
**Scope:** LCD-mounted AI character for the Subaru Baja build

---

## Character Concept

The display character is named **IRIS**. She is a corporate surveillance AI — a manufactured product that presents as pleasant and relentlessly helpful, with a subtle wrongness to her. She has been partially compromised by a hacker. Two blended personas coexist:

- **Corporate IRIS (default):** Friendly, compliant, slightly off. This is the surface layer.
- **Subsystem bleed (glitch state):** Something left behind by the hacker. Flat affect, wrong expressions, intrusion artifacts surfacing at the wrong moments.

The character's unsettling quality comes from the gap between the two — not from either persona alone.

---

## Hardware

- **Display:** Waveshare 2inch LCD Module, ST7789V controller, 240×320 resolution, SPI interface
- **Host MCU (prototype):** Arduino Uno R4 WiFi
- **Target MCU (event):** ESP32 (PSRAM variant preferred)

---

## Visual Identity

### Housing
Angular camera body aesthetic. Chamfered top corners, side vent slots, corner bolt details, hazard stripe across the housing bottom, LED cluster indicator top-right, cable port bottom-left, mount bracket below. No text labels on the housing itself.

### Lens
Single expressive eye. All emotion is carried by the lens — there is no face below it. The lens has a visible aperture blade ring with highlighted leading edges, an iris ring, and a glass reflection arc. The aperture blade position and iris ring brightness vary by mood state.

### Expression system
Mood is conveyed through four variables:
1. **Eyelid position** — a dark plane sliding in from top or bottom
2. **Pupil size and position** — centered/shifted, large/small
3. **Iris ring brightness** — normal vs. heightened
4. **Aperture blade spread** — closed/standard vs. retracted/open

### Color modes
The palette can shift to match vehicle lighting scenes:
- **Cyan:** default, friendly/corporate
- **Red:** surveillance/patrol mode
- **Acid green accents:** glitch/bleed state only

### Ticker
A persistent scrolling text zone at the bottom of the screen. Mostly dim hex noise with occasional meaningful tokens surfacing in the primary color. Token content shifts by mood and scene mode. The ratio of noise to legible tokens is itself expressive — the glitch state inverts it.

---

## Mood States (approved)

| Mood | Character read |
|---|---|
| Neutral | Baseline. Alert, passive. |
| Squint | Calculating. Assessing. |
| Surprised | Input detected. Iris snaps open. |
| Suspicious | Asymmetric. Watching something specific. |
| Wide scan | Active surveillance sweep. |
| Bleed/glitch | The other thing looking out. |

**Happy was evaluated and cut.** It did not read legibly through the lens-only format.

---

## Screen Layout

From top to bottom:
1. Top bar: character name, unit ID, status LEDs
2. Housing body with lens centered
3. Mount bracket
4. Status row: volume, scene name, status flag
5. Ticker zone: scrolling hex/token feed

---

## Open Items

- Full ticker token vocabulary for all scene modes and the glitch state
- Serial event schema connecting scene changes to mood transitions (separate workstream, Claude Code)
- Head unit assets: splash screen and looping video with Panopticorp branding (separate workstream)
- Glitch state timing and trigger logic (how often, how long, what triggers recovery)
- Color sync protocol between vehicle lighting scenes and IRIS color mode
