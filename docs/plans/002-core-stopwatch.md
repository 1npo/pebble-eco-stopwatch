# 002 — Core stopwatch (vertical slice)

Phase 1 of 3. Follow-ups: `003-laps-and-animation.md`, `004-settings-and-themes.md`.
Spec: `000-initial-request.md`. This document is also the **reference** for the platform matrix, layout constants, and module layout used by 003/004.

## Summary

1. Eco Stopwatch (repo/folder `pebble-eco-stopwatch`; called "Eco Timer" in `000`/early drafts) is a stopwatch watchapp whose screen redraws only every N seconds (default 10) while running, plus once immediately on each button press.
2. Elapsed time is always computed from wall-clock ms (`time_ms()`) and never from counted ticks, so it stays exact at any refresh cadence and while the app is closed.
3. One `app_timer` drives refreshes. It is re-armed to fire on the next multiple of the interval of *elapsed* time, so the running display reads :00, :10, :20… We use no `tick_timer_service`, no per-second wakeups, and no timers while stopped/paused/unfocused.
4. Timer and subtext are drawn by one custom `Layer` with Iosevka Heavy (timer) and Iosevka Narrow Regular (subtext), pre-rendered at two size groups.
5. The right-hand action bar is a custom `Layer` of standard `ACTION_BAR_WIDTH`, with icons drawn from primitives. There are no bitmaps, and theme colours apply trivially.
6. Phase 002 delivers STOPPED/RUNNING/PAUSED, reset, the refresh strategy, app-focus handling, state persistence across app exit, and all 7 platforms.
7. Phase 003 adds laps, the lap-reveal animation, and lap scrolling.
8. Phase 004 adds Clay settings (`@rebble/clay`), live re-arming, the three themes, and the README.
9. The fonts were already converted with this SDK (4.33.1) for all 7 platforms. Each is about 2 KB, and aplite has more than 23 KB of heap free.
10. Every step ends in a build/emulator checkpoint. Emulator commands must run **outside the Claude Code sandbox** (see Tooling notes). The user tests on their Pebble Time 2 (emery) via `pebble install --cloudpebble`; emery is the primary target, and round screens are best-effort.

## Decisions & assumptions

| # | Decision | Why |
|---|----------|-----|
| D1 | "Exceeds 99:99:99" means **hours ≥ 100** (display would need a 3rd hour digit). Shrunk font applies from 100:00:00. Display clamps at **999:59:59**. | 99:99:99 isn't a valid time; the shrink exists to fit a 9th character. |
| D2 | Refresh mechanism: a single one-shot `app_timer_register`, re-armed in its own callback with `delay = interval_ms − (elapsed_ms % interval_ms) + 15`. | Only one wakeup per interval. Aligning to elapsed boundaries makes the shown seconds exact at draw time. +15 ms slack guards against firing a hair early and showing :09. |
| D3 | No `tick_timer_service`. | SECOND_UNIT wakes every second (10× too often). MINUTE_UNIT is aligned to the wall clock, not to elapsed time. |
| D4 | No wakeups (`wakeup_*`) and no background worker. | The stopwatch keeps "running" while closed because state is a start timestamp. Nothing needs to execute while closed. |
| D5 | Immediate redraw on every button action, on focus regain, and on settings change. Pausing shows the exact elapsed seconds (not an interval multiple). | The user expects feedback on a press. The paused value should be exact. |
| D6 | When the app loses focus (notification overlay), the refresh timer is cancelled. On regain, redraw and re-arm. | Drawing under an overlay is wasted work. |
| D7 | Persist state **once, in deinit** (app exit), and restore it in init. A RUNNING timer stays running across exit: its wall-clock `start_ms` is persisted. | One flash write per session. Exit always runs deinit. A crash loses at most the current session's changes (acceptable). |
| D8 | Backlight: never touched (no `light_*` calls). | The system already lights on button presses per user prefs, and forcing it costs a lot of power. |
| D9 | Vibration: none. | Not in the spec, and it costs power. |
| D10 | Custom-drawn action bar (`Layer`, width `ACTION_BAR_WIDTH`), not `ActionBarLayer`. | Iosevka has ▶⏸⟲▲▼ but not 🏁, and glyph icons at 14–18 px are poor. A custom bar draws crisp primitives in any theme colour on B&W and colour, needs no bitmap resources, and gives full control of round-screen icon insets. The cost is the loss of the system icon-press nudge animation (irrelevant to battery). |
| D11 | The refresh-interval icon is drawn with primitives (clockwise ring with an arrowhead), not the font's ⟳ glyph. | The glyph renders as an almost-plain circle at 14 px (checked). |
| D12 | The interval label is rendered as **`10s`** (number + lowercase `s`), not `10 SEC`. This is the one deliberate exception to "subtext is capitalized". **Approved by the user.** Lowercase `s` has the same advance as `S` (7 px @14, 9 px @18, measured), so widths are unchanged. | Measured: `STOPPED` + icon + `60 SEC` needs ~100 px, but the 144-wide line is 102 px. `10s` leaves a ≥20 px gap everywhere. |
| D13 | Allowed refresh intervals: **1, 2, 5, 10, 15, 20, 30, 60 s** (default 10). Out-of-set values snap to 10. **Approved by the user.** | Divisors of 60 keep running displays on round values. A bounded set prevents silly values. A Clay `select` is clearer than a slider. |
| D14 | Fonts use the **hinted `TTF/`** variants. | Compared both in monochrome at 14 px. Hinted stems are slightly more even, and widths are identical. |
| D15 | Two font size groups selected at compile time. Small (aplite, basalt, diorite, flint, chalk): timer 26 / shrunk 22, text 14. Large (emery, gabbro): timer 38 / shrunk 34, text 18. | Width-limited by 8 monospace digits (`00:00:00` = 4 em). The sizes were measured with the SDK's own FreeType pipeline (tables below). |
| D16 | "Timer centered" = the timer+subtext **block** is centred vertically in the screen and horizontally on `CONTENT_CX` (the centre of the area left of the bar; tuned per round screen). | The bar occupies the right edge, so centring on the full width would crowd it. |
| D17 | Default theme: **Color** on colour platforms, **White on Black** on B&W. | The owner's watch is colour, and Color is the most informative. Implemented in 004. In 002 the default is hard-coded the same way. |
| D18 | **Don't modify `wscript`.** Every build command is `unset CFLAGS && pebble build` (one Bash call, because each Claude Code Bash call starts a fresh shell from the user's profile). | The user's shell exports `CFLAGS=-O3 -ffast-math -march=native`, which makes `arm-none-eabi-gcc` fail (`unrecognized -march target: native`). The user confirmed `unset CFLAGS` is safe and is the preferred fix. |
| D20 | `model_now_ms()` is **monotonic**: it returns `max(raw, last_returned)`. | In the emulator, `time_ms()` seconds and ms were observed out of step (a later read was ~1 s *earlier*). A monotonic clamp costs one compare and stops the display ever stepping backwards. |
| D19 | Settings library is **`@rebble/clay` 1.1.0**, not `pebble-clay`. | `pebble-clay` 1.0.4 fails the build: "The Pebble package 'pebble-clay' doesn't support the 'gabbro' platform". `@rebble/clay` builds for all 7 and its `BW`/`COLOR` capability maps include flint/gabbro (verified). |

## Open risks

- **R1 Round layout feel (chalk/gabbro): low priority.** Positions are computed geometrically. Round screens must build and run without clipping-induced crashes, but their screenshots are best-effort and never block a checkpoint. If something looks off, adjust only the `config.h` constants for that platform.
- **R2 Thin subtext.** Narrow Regular has 1 px stems. The user will judge it on their Time 2 and may switch fonts. Keep the text font isolated to the `FONT_TEXT_*` resources and `RES_FONT_TEXT` so a swap touches only `package.json` (and possibly `config.h` cap metrics).
- **R3 Emulator flakiness.** The first `pebble install --emulator X` after a cold boot can time out (`libpebble2.exceptions.TimeoutError`). Re-run once. Inside the Claude Code sandbox it *always* times out (localhost TCP is blocked), so run emulator commands with the sandbox disabled or ask the user to run them.
- **R4 Font folder size.** `resources/fonts/` holds 64 TTFs (~110 MB). Only two are used. Recommend the user commit only those two (see step 1); don't delete anything without asking.

## Platform matrix (reference for all phases)

All values were derived from the SDK (`pebble_sdk_platform.py`, `pebble.h`) and from glyph metrics measured with the SDK FreeType pipeline. "ink" = actual lit pixels. With this SDK, Pebble draws a glyph at `box_y + (em − glyph_top)`, verified in an emery screenshot: the T38 box at y=20 has its first ink row at y=30.

| | aplite | diorite | flint | basalt | chalk | emery | gabbro |
|---|---|---|---|---|---|---|---|
| Display | 144×168 rect | 144×168 rect | 144×168 rect | 144×168 rect | 180×180 round | 200×228 rect | 260×260 round |
| Colour | B&W | B&W | B&W | 64 col | 64 col | 64 col | 64 col |
| App RAM | 24 KB | 64 KB | 64 KB | 64 KB | 64 KB | 128 KB | 128 KB |
| `ACTION_BAR_WIDTH` | 30 | 30 | 30 | 30 | 40 | 34 | 40 |
| Font group | S | S | S | S | S | L | L |
| Lap rows visible (003) | 6 | 6 | 6 | 6 | 4 | 7 | 6 |

Font group metrics (em size → ink offset from box top / ink height; widths in px):

| Group | Timer font | ink top / h | `00:00:00` ink w | Shrunk font | ink top / h | `000:00:00` ink w | Text font | cap top / h |
|---|---|---|---|---|---|---|---|---|
| S | Heavy 26 | 7 / 19 | 102 (adv 104) | Heavy 22 | 5 / 17 | 97 (adv 99) | Narrow Reg 14 | 3 / 11 |
| L | Heavy 38 | 10 / 28 | 148 (adv 152) | Heavy 34 | 9 / 25 | 149 (adv 153) | Narrow Reg 18 | 4 / 14 |

Layout constants (`config.h`, window coordinates unless noted). These are final values for the implementer; derivations are in comments.

| Constant | rect 144×168 | chalk | emery | gabbro | Meaning |
|---|---|---|---|---|---|
| `CONTENT_CX` | 57 | 82 | 83 | 130 | x centre of timer/subtext/laps column |
| `LINE_W` | 102 | 102 | 148 | 148 | subtext line & rule width (= big timer ink width) |
| `TIMER_BOX_H` | 34 | 34 | 46 | 46 | height of timer text box (em + 8) |
| `TIMER_SMALL_DY` | 3 | 3 | 2 | 2 | y offset for shrunk font so ink stays centred |
| `SUB_Y` (head-local) | 29 | 29 | 42 | 42 | subtext box top inside head layer |
| `SUB_BOX_H` | 20 | 20 | 24 | 24 | subtext box height (em + 6) |
| `HEAD_H` | 49 | 49 | 66 | 66 | head layer height (`SUB_Y + SUB_BOX_H`) |
| `HEAD_Y_CENTER` | 59 | 65 | 79 | 95 | head layer y when no laps |
| `HEAD_Y_LAPS` | 0 | 21 | 0 | 19 | head layer y when laps shown (003) |
| `RULE_Y` | 48 | 69 | 66 | 85 | laps layer y = rule line y (003) |
| `RULE_THICK` | 1 | 1 | 2 | 2 | |
| `ROW0_Y` (laps-local) | 3 | 3 | 4 | 4 | first lap row box top inside laps layer (003) |
| `ROW_PITCH` | 17 | 17 | 22 | 22 | (003) |
| `LAP_ROWS_VISIBLE` | 6 | 4 | 7 | 6 | (003) |
| `ICON_SZ` | 15 | 15 | 19 | 19 | action-bar icon bounding size |
| `BAR_ICON_X` (centre) | 129 | 158 (select 160) | 183 | 236 (select 240) | |
| `BAR_ICON_Y` up/select/down | 28/84/140 | 50/90/130 | 38/114/190 | 75/130/185 | |
| `REFRESH_ICON_D` | 11 | 11 | 14 | 14 | refresh icon diameter (= cap height) |

Checks already done: on round screens every corner of the timer ink, the subtext line and each lap row lies ≥4 px inside the circle and ≥6 px left of the bar. On rect, the timer ink has 6 px (S) / 9 px (emery) clear on each side, and the right gap to the bar is the same. Lap rows were checked with the widest string `LAP 99  999:59:59`.

**Text drawing rule:** always draw single lines with `GTextOverflowModeTrailingEllipsis` and a box at least `em + 6` tall. With `GTextOverflowModeFill`, a too-narrow box wraps at the double space (seen on aplite in the prototype).

## Tooling notes

- Build: **always** `unset CFLAGS && pebble build` in a single command.
- Hardware: the **user** installs on their Pebble Time 2 with `pebble install --cloudpebble`. The implementer never runs this; at the end of each phase it tells the user that a build is ready to try.
- Emulator: `pebble install --emulator <platform>`, `pebble screenshot --emulator <platform> --no-open <file>.png`, `pebble logs --emulator <platform>`, `pebble emu-button --emulator <platform> click select|up|down|back`, `pebble emu-button ... push select` / `release select` (long press), `pebble emu-set-time --emulator <platform> <unix-seconds>`, `pebble kill`.
- **Claude Code:** emulator commands need `dangerouslyDisableSandbox: true` (or the user runs them with `! <cmd>`). If a command fails with `libpebble2.exceptions.TimeoutError`, re-run it once outside the sandbox before debugging.
- Read screenshots with the Read tool (PNG) to check layouts visually.

## Files (whole project, all phases)

```
.gitignore                   (create)                            002
package.json                 (modify: resources, messageKeys, dependency) 002/004
src/c/pebble-eco-stopwatch.c (delete; replaced by main.c)        002
src/c/config.h               platform constants, limits, enums   002
src/c/main.c                 init/deinit, window, focus service  002
src/c/timer_model.{c,h}      state, elapsed, laps array          002 (+laps 003)
src/c/format.{c,h}           hh:mm:ss formatting                 002
src/c/refresh.{c,h}          the one app_timer                   002
src/c/ui.{c,h}               layers + update procs               002 (+laps 003)
src/c/icons.{c,h}            primitive icon drawing              002
src/c/controller.{c,h}       state machine, click config         002 (+003)
src/c/theme.{c,h}            colours                             002
src/c/settings.{c,h}         interval/theme values + persistence 002 (defaults) / 004 (AppMessage)
src/c/persist.{c,h}          timer + laps persistence            002 (+laps 003)
src/c/anim.{c,h}             lap reveal/hide animation           003
src/pkjs/index.js            Clay bootstrap                      004
src/pkjs/config.js           Clay config                         004
README.md                    (rewrite)                           004
```

Every `.c` file includes `<pebble.h>` and its own header. Headers use `#pragma once`. All module state is `static` in its `.c` file and exposed only through functions. Use the existing scaffold's style: 2-space indent and `prv_` prefix for static functions.

## Steps

### Step 1 — Toolchain, resources, skeleton (risk: font pipeline, build env)

1. Confirm you are in `~/dev/projects/pebble-eco-stopwatch` and that `package.json` has `"name": "pebble-eco-stopwatch"`, `"displayName": "Eco Stopwatch"` (the rename was already done; don't change the UUID).
2. Create `.gitignore`:
   ```
   build/
   node_modules/
   .lock-waf*
   *.pyc
   ```
3. `package.json` → `pebble.resources.media` (paths relative to `resources/`):
   ```json
   [
     {"type":"font","name":"FONT_TIMER_26","file":"fonts/IosevkaCustomForPebble/TTF/IosevkaCustomForPebble-Heavy.ttf","characterRegex":"[0-9:]","targetPlatforms":["aplite","basalt","chalk","diorite","flint"]},
     {"type":"font","name":"FONT_TIMER_22","file":"fonts/IosevkaCustomForPebble/TTF/IosevkaCustomForPebble-Heavy.ttf","characterRegex":"[0-9:]","targetPlatforms":["aplite","basalt","chalk","diorite","flint"]},
     {"type":"font","name":"FONT_TEXT_14","file":"fonts/IosevkaCustomForPebbleNarrow/TTF/IosevkaCustomForPebbleNarrow-Regular.ttf","characterRegex":"[ 0-9:A-Zs]","targetPlatforms":["aplite","basalt","chalk","diorite","flint"]},
     {"type":"font","name":"FONT_TIMER_38","file":"fonts/IosevkaCustomForPebble/TTF/IosevkaCustomForPebble-Heavy.ttf","characterRegex":"[0-9:]","targetPlatforms":["emery","gabbro"]},
     {"type":"font","name":"FONT_TIMER_34","file":"fonts/IosevkaCustomForPebble/TTF/IosevkaCustomForPebble-Heavy.ttf","characterRegex":"[0-9:]","targetPlatforms":["emery","gabbro"]},
     {"type":"font","name":"FONT_TEXT_18","file":"fonts/IosevkaCustomForPebbleNarrow/TTF/IosevkaCustomForPebbleNarrow-Regular.ttf","characterRegex":"[ 0-9:A-Zs]","targetPlatforms":["emery","gabbro"]}
   ]
   ```
   The pixel size comes from the number in the resource name (SDK rule). Do **not** add `compress` (RLE4) or `compatibility`. Leave `messageKeys` as `["dummy"]` until 004.
4. Delete `src/c/pebble-eco-stopwatch.c` (scaffold sample). Create `src/c/config.h` containing:
   - `#if PBL_DISPLAY_WIDTH == 144` / `#elif defined(PBL_PLATFORM_CHALK)` / `#elif defined(PBL_PLATFORM_EMERY)` / `#elif defined(PBL_PLATFORM_GABBRO)` / `#else #error "unsupported platform"` blocks defining every constant in the layout table above. Also define `RES_FONT_TIMER`, `RES_FONT_TIMER_SMALL`, `RES_FONT_TEXT` as the matching `RESOURCE_ID_FONT_*`.
   - `SCREEN_W PBL_DISPLAY_WIDTH`, `SCREEN_H PBL_DISPLAY_HEIGHT`, `BAR_W ACTION_BAR_WIDTH`.
   - `#define MAX_LAPS 99`, `#define MAX_ELAPSED_S 3599999` (999:59:59), `#define SHRINK_AT_S 360000` (100 h), `#define REFRESH_SLACK_MS 15`.
   - `#define ECO_DEBUG_LOG 0` and `#define ECO_LOG(...)` that expands to `APP_LOG(APP_LOG_LEVEL_DEBUG, __VA_ARGS__)` when it is 1, else to nothing.
5. Create a minimal `main.c` that loads the three fonts with `fonts_load_custom_font(resource_get_handle(RES_FONT_*))`, pushes a black window, and draws `00:00:00` centred at `CONTENT_CX` in a box at `HEAD_Y_CENTER`. Unload the fonts with `fonts_unload_custom_font` in deinit.

**Checkpoint 1:** `unset CFLAGS && pebble build` succeeds for all 7 platforms. The memory report for APLITE shows heap free > 20000 bytes. `pebble install --emulator aplite` and then `emery`, each followed by a screenshot: the digits are visible and not clipped.

*Tell the user (don't do it yourself):* only `resources/fonts/IosevkaCustomForPebble/TTF/IosevkaCustomForPebble-Heavy.ttf` and `resources/fonts/IosevkaCustomForPebbleNarrow/TTF/IosevkaCustomForPebbleNarrow-Regular.ttf` are referenced. Committing the other 62 TTFs adds about 110 MB to git.

### Step 2 — Timer model, formatting, persistence (pure logic)

`timer_model.h`:
```c
typedef enum { TS_STOPPED = 0, TS_RUNNING = 1, TS_PAUSED = 2 } TimerState;

void       model_init(void);               // zeroed STOPPED
TimerState model_state(void);
int64_t    model_now_ms(void);             // time_ms(&s,&ms) -> (int64_t)s*1000 + ms, clamped monotonic (D20)
int64_t    model_elapsed_ms(void);         // accum + (RUNNING ? max(0, now - start) : 0)
uint32_t   model_elapsed_s(void);          // min(elapsed_ms/1000, MAX_ELAPSED_S)
void       model_start(void);              // STOPPED/PAUSED -> RUNNING: start_ms = now
void       model_pause(void);              // RUNNING -> PAUSED: accum += max(0, now - start)
void       model_reset(void);              // any -> STOPPED: zero everything (laps too, in 003)
// persistence access
void       model_get_snapshot(TimerState *st, int64_t *start_ms, int64_t *accum_ms);
void       model_restore(TimerState st, int64_t start_ms, int64_t accum_ms);
```
Internal struct: `static struct { TimerState state; int64_t start_ms; int64_t accum_ms; } s_m;`. Never accumulate per tick. Clamp negative segments (wall clock moved backwards) to 0.

`format.h`: `void format_hms(char *buf, size_t len, uint32_t total_s);`. Clamp to `MAX_ELAPSED_S`, then `snprintf(buf, len, "%02d:%02d:%02d", h, m, s)` with `int` casts. Hours ≥ 100 print 3 digits naturally. Buffers are `char[12]`.

`persist.h`: `void persist_load_all(void); void persist_save_all(void);`. Keys and schema (shared with 003/004; define all of them now):
```c
#define PKEY_SCHEMA    1   // int, value PERSIST_SCHEMA_VERSION (1)
#define PKEY_TIMER     2   // data, PersistTimer
#define PKEY_LAPS_A    3   // data, uint32_t[<=50]  laps 1..50   (003)
#define PKEY_LAPS_B    4   // data, uint32_t[<=49]  laps 51..99  (003)
#define PKEY_INTERVAL  5   // int, seconds          (004)
#define PKEY_THEME     6   // int, Theme            (004)

typedef struct {
  uint8_t state;       // TimerState
  uint8_t lap_count;   // 0..99 (003; write 0 in 002)
  uint8_t reserved[6];
  int64_t start_ms;
  int64_t accum_ms;
} PersistTimer;         // 24 bytes
```
Load: if `persist_read_int(PKEY_SCHEMA) != 1`, keep defaults (and do not delete anything). Otherwise read `PersistTimer` with `persist_read_data`, validate (`state <= 2`, `accum_ms >= 0`), and call `model_restore`. Save: write the schema and the timer struct.

**Checkpoint 2:** builds. Temporarily call `model_start()` in init and `ECO_LOG` the result of `format_hms` 3 s later (via a one-off `app_timer`). The log shows `00:00:03`. Remove the temporary code.

### Step 3 — UI layers, theme, icons (risk: round layout, primitive icons)

`theme.h`:
```c
typedef enum { THEME_WOB = 0, THEME_BOW = 1, THEME_COLOR = 2 } Theme;
void   theme_set(Theme t);      // on PBL_BW, THEME_COLOR is stored but behaves as THEME_WOB
GColor theme_bg(void);          // WOB/COLOR: GColorBlack, BOW: GColorWhite
GColor theme_fg(void);          // WOB/COLOR: GColorWhite, BOW: GColorBlack
GColor theme_bar_bg(void);      // = theme_fg()  (bar is the inverse block, like the system bar)
GColor theme_bar_fg(void);      // = theme_bg()
GColor theme_verb(TimerState s);// COLOR on PBL_COLOR: STOPPED GColorRed, PAUSED GColorYellow,
                                //   RUNNING GColorGreen; otherwise theme_fg()
```
Use `#if defined(PBL_COLOR)` around the colour branch.

`settings.h` (002 version, extended in 004): `void settings_load(void); uint8_t settings_interval_s(void); Theme settings_theme(void);`. In 002, `settings_load` only sets defaults: interval 10, theme `PBL_IF_COLOR_ELSE(THEME_COLOR, THEME_WOB)`.

`icons.h` (all take centre `c`, size `s` = `ICON_SZ`, and use the context's current fill colour; the caller sets `graphics_context_set_fill_color(ctx, theme_bar_fg())`):
```c
void icon_play(GContext *ctx, GPoint c, int16_t s);    // filled triangle (c.x-s/3, c.y-s/2) (c.x-s/3, c.y+s/2) (c.x+s/2, c.y)
void icon_pause(GContext *ctx, GPoint c, int16_t s);   // two bars w=s/4, h=s, gap=s/4, centred
void icon_up(GContext *ctx, GPoint c, int16_t s);      // filled triangle apex (c.x, c.y-s/3), base y=c.y+s/3, x=c.x±s/2
void icon_down(GContext *ctx, GPoint c, int16_t s);    // mirror of up
void icon_flag(GContext *ctx, GPoint c, int16_t s, GColor bg); // see below
void icon_cycle(GContext *ctx, GPoint c, int16_t d, int16_t thick, bool clockwise); // see below
void icons_init(void); void icons_deinit(void);         // create/destroy the 3-point GPaths
```
- Triangles: create one `GPath` per shape in `icons_init` from static `GPathInfo` (3 points relative to (0,0)) for **each** size used, then `gpath_move_to(path, c)` and `gpath_draw_filled`. Do not allocate per draw.
- `icon_flag`: pole = `GRect(c.x - s/2, c.y - s/2, 2, s)`. The flag is a grid of 4 cols × 3 rows with cell size `k = (s - 3) / 4` (3 for s=15, 4 for s=19), top-left at `(c.x - s/2 + 2, c.y - s/2)`. Fill cells where `(row + col) % 2 == 0` with the fill colour. Then draw a 1 px outline around the whole grid with `graphics_draw_rect` in the same colour (set the stroke colour to the fill colour). The `bg` parameter is unused for now; keep it for future contrast tweaks.
- `icon_cycle`: ring via `graphics_fill_radial(ctx, GRect(c.x - d/2, c.y - d/2, d, d), GOvalScaleModeFitCircle, thick, a0, a1)` (0° = 12 o'clock, clockwise, `DEG_TO_TRIGANGLE`; verified in `pebble.h`).
  - Counter-clockwise (⟲, reset): `a0 = 0°, a1 = 300°` (gap at top-left). Arrowhead: a filled triangle at the top pointing **left**: tip `(c.x - 3, c.y - d/2 + thick/2)`, base points `(c.x + 1, c.y - d/2 - 2)` and `(c.x + 1, c.y - d/2 + thick + 2)`.
  - Clockwise (refresh): `a0 = 60°, a1 = 360°` (gap at top-right). Arrowhead mirrored about `c.x`, pointing **right**.
  - Draw arrowheads with a GPath created in `icons_init`, like the triangles.
  - Reset icon: `d = ICON_SZ`, `thick = 2` (S) / `3` (L). Refresh icon: `d = REFRESH_ICON_D`, `thick = 1` (S) / `2` (L).

`ui.h`:
```c
void ui_create(Window *w);       // creates head, bar (and laps in 003) layers
void ui_destroy(void);
void ui_mark_dirty(void);        // marks the head layer dirty (the whole window re-renders anyway)
void ui_apply_theme(void);       // window_set_background_color + mark all dirty
void ui_on_interval_changed(void); // recompute cached interval-label width, mark dirty
Layer *ui_head_layer(void);      // for anim.c (003)
```
Layer tree (add in this order so the bar is on top): `root → head_layer → (laps_layer, 003) → bar_layer`.
- `head_layer` frame: `GRect(0, HEAD_Y_CENTER, 2 * CONTENT_CX, HEAD_H)`. Update proc:
  1. `s = model_elapsed_s()`, then `format_hms`.
  2. Font and y: `s >= SHRINK_AT_S` → (`small font`, `TIMER_SMALL_DY`), else (`timer font`, 0).
  3. `graphics_draw_text(text, font, GRect(0, y, 2*CONTENT_CX, TIMER_BOX_H), TrailingEllipsis, GTextAlignmentCenter, NULL)` in `theme_fg()`.
  4. Subtext line rect `L = GRect(CONTENT_CX - LINE_W/2, SUB_Y, LINE_W, SUB_BOX_H)`. Draw the verb `"STOPPED" | "RUNNING" | "PAUSED"` left-aligned in `theme_verb(state)`. Draw the interval label `"%ds"` (lowercase `s`) right-aligned in `theme_fg()`.
  5. Refresh icon: centre `x = L.origin.x + LINE_W - label_w - 3 - REFRESH_ICON_D/2`, `y = SUB_Y + cap_top + REFRESH_ICON_D/2` (cap_top 3 S / 4 L). `label_w` comes from `graphics_text_layout_get_content_size`, cached in a static and recomputed only when the interval changes (`ui_on_interval_changed()`).
- `bar_layer` frame: `GRect(SCREEN_W - BAR_W, 0, BAR_W, SCREEN_H)`. The update proc fills the bounds with `theme_bar_bg()` (round screens clip it to the circle naturally). Then it draws the icons for the current UI mode at `(BAR_ICON_X - (SCREEN_W - BAR_W), BAR_ICON_Y_*)` (convert to layer-local). For round screens use the separate select-x from the matrix. The icon table is in step 4.

**Checkpoint 3:** with a hard-coded STOPPED state, build, install, and screenshot `emery`, `basalt`, `aplite` (gating), plus `chalk` and `gabbro` (best-effort, R1).
- Accept: digits are centred on `CONTENT_CX`; ≥6 px is clear between the digit ink and the bar; the subtext is fully visible with ≥10 px between the verb and the icon; on round screens nothing touches the bezel.
- Temporarily force the bar to draw all 7 icons (3 at a time, two builds) and check each is recognisable on emery and aplite.
- Temporarily set `accum_ms` to 100 h and check the shrunk font fits the same margins.
- Remove all temporary code.

### Step 4 — State machine, clicks, refresh (core behaviour)

`controller.h`:
```c
typedef enum { UI_STOPPED, UI_RUNNING, UI_PAUSED, UI_PAUSED_SCROLL } UiMode;  // PAUSED_SCROLL used in 003
UiMode controller_mode(void);
void   controller_init(Window *w);       // window_set_click_config_provider(w, prv_click_config)
void   controller_on_focus(bool in_focus);
void   controller_on_settings_changed(void);  // 004
```
`controller_mode()`: STOPPED→`UI_STOPPED`, RUNNING→`UI_RUNNING`, PAUSED→`UI_PAUSED` (003 adds `UI_PAUSED_SCROLL` when `lap_count > LAP_ROWS_VISIBLE`).

Button table (UP = button 1, SELECT = button 2, DOWN = button 3). "—" = no icon and the click is ignored:

| Mode | UP | SELECT | DOWN |
|---|---|---|---|
| UI_STOPPED | — | ▶ Start → RUNNING | — |
| UI_RUNNING | 🏁 Lap (003; in 002 the icon is shown and the click ignored) | ⏸ Pause → PAUSED | ⟲ Reset → STOPPED |
| UI_PAUSED | — | ▶ Resume → RUNNING | ⟲ Reset → STOPPED |
| UI_PAUSED_SCROLL (003) | ▲ scroll up | ▶ Resume | ▼ scroll down (no reset in this mode; resume first) |

Every transition does exactly this, in order:
1. call the model function;
2. `refresh_reschedule()` (it decides by itself whether to arm);
3. `layer_mark_dirty(bar)` and `ui_mark_dirty()` (immediate redraw).

Click config: `window_single_click_subscribe` for UP, SELECT, DOWN, each dispatching on `controller_mode()`. Back is left to the system (exits the app).

`refresh.h`: `void refresh_reschedule(void); void refresh_cancel(void); void refresh_set_focus(bool in_focus);`
```c
static AppTimer *s_timer; static bool s_focused = true;
void refresh_reschedule(void) {
  refresh_cancel();
  if (model_state() != TS_RUNNING || !s_focused) return;
  uint32_t iv = settings_interval_s() * 1000;
  uint32_t phase = (uint32_t)(model_elapsed_ms() % iv);
  s_timer = app_timer_register(iv - phase + REFRESH_SLACK_MS, prv_fire, NULL);
}
static void prv_fire(void *ctx) { s_timer = NULL; ECO_LOG("refresh"); ui_mark_dirty(); refresh_reschedule(); }
void refresh_cancel(void) { if (s_timer) { app_timer_cancel(s_timer); s_timer = NULL; } }
```

`main.c`:
- init: `settings_load(); persist_load_all(); theme_set(settings_theme()); icons_init();` → create the window (load handler: `ui_create`, `ui_apply_theme`; unload: `ui_destroy`) → `controller_init` → `window_stack_push(w, true)` → `app_focus_service_subscribe_handlers((AppFocusHandlers){ .did_focus = prv_did_focus })` → `refresh_reschedule()`.
- `prv_did_focus(bool in_focus)`: `refresh_set_focus(in_focus)`. If in focus: `ui_mark_dirty(); refresh_reschedule();` else `refresh_cancel()`.
- deinit: `refresh_cancel(); persist_save_all(); app_focus_service_unsubscribe(); window_destroy; icons_deinit(); fonts unload`.

**Checkpoint 4** (emery, then aplite):
- [ ] Launch → `00:00:00 STOPPED`, ▶ only.
- [ ] `emu-button click select` → RUNNING, ▶ becomes ⏸, 🏁 and ⟲ appear, and the display is redrawn at once.
- [ ] With `ECO_DEBUG_LOG 1`, `pebble logs` over 60 s running shows ~6 `refresh` lines at 10 s spacing, and the screenshot seconds are multiples of 10.
- [ ] Select → PAUSED shows the exact seconds (e.g. `00:00:27`). Logs show no `refresh` lines for 30 s.
- [ ] Select → RUNNING continues from the paused value (no jump).
- [ ] Down → STOPPED `00:00:00`. Down in RUNNING also resets.
- [ ] Running, press back (exit), wait 20 s, relaunch: still RUNNING, and the value includes the 20 s.
- [ ] Paused, exit, relaunch: PAUSED with the same value.
- [ ] Running: `pebble emu-set-time --emulator emery <now + 360000>`. The next refresh shows ≥100:00:00 in the small font, inside the margins.
- [ ] Set `ECO_DEBUG_LOG 0` before finishing.

### Step 5 — All-platform pass

Install and screenshot STOPPED and RUNNING on `aplite`, `diorite`, `flint`, `basalt`, `emery` (gating) and `chalk`, `gabbro` (best-effort). Fix only `config.h` constants if anything clips. Done when the gating screenshots pass the Checkpoint 3 acceptance rules. Then tell the user the build is ready for `pebble install --cloudpebble` on the Time 2.

## Done criteria (phase 002)

- `pebble build` is clean for all 7 platforms (only the linker's RWX-segment warning, which comes from the SDK).
- Checkpoint 4 passes on emery and aplite. Step 5 gating screenshots pass; round screens run without crashing.
- While RUNNING there is exactly one wakeup per interval. While STOPPED, PAUSED or unfocused there are none.
- No `tick_timer_service`, `light_*`, `vibes_*` or `wakeup_*` calls anywhere.
