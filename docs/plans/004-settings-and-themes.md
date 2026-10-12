# 004 — Clay settings, themes, README

Phase 3 of 3. **Prerequisite:** `003-laps-and-animation.md` is done. Constants, persist keys (`PKEY_INTERVAL`, `PKEY_THEME`) and tooling notes are in `002-core-stopwatch.md`.

## Goal

A phone-side settings page (Clay) sets the refresh interval and the colour theme. Changes apply live, including to a running timer, and persist on the watch. On B&W watches the page hides the Color option. The README documents the app.

## Decisions & assumptions

| # | Decision | Why |
|---|----------|-----|
| S1 | Use **`@rebble/clay` ^1.1.0**, installed with `pebble package install @rebble/clay`. | `pebble-clay` 1.0.4 refuses to build for flint/gabbro (verified). `@rebble/clay` 1.1.0 builds for all 7 (verified). |
| S2 | Message keys: `RefreshInterval`, `Theme` (they replace `dummy`). | — |
| S3 | Refresh interval control: `select` with 1, 2, 5, 10, 15, 20, 30, 60 s (labels `"1 second"`, `"2 seconds"`, …), default `"10"`. | See 002 D13. |
| S4 | Theme: **two `select` items with the same `messageKey: "Theme"`**: one with `capabilities: ["COLOR"]` (3 options, default `"2"`), one with `capabilities: ["BW"]` (2 options, default `"0"`). | Documented Clay feature ("multiple items with the same messageKey as long as they do not both satisfy the same conditions"). In `@rebble/clay`, `BW` = aplite/diorite/flint and `COLOR` = basalt/chalk/emery/gabbro (verified in `src/scripts/lib/utils.js`). |
| S5 | Clay `select` values arrive on the watch as **C strings** (Clay passes strings through unchanged). The watch parser accepts `TUPLE_CSTRING`, `TUPLE_INT` and `TUPLE_UINT`. | Robust to either encoding. `atoi` isn't declared in `pebble.h`, so use a tiny digit parser. |
| S6 | Settings are persisted on receipt (`persist_write_int`), not at exit. | Rare event, and it must survive a crash. |
| S7 | Clay uses its default auto-handling (`new Clay(config)`); no custom JS. | The capabilities in S4 make custom filtering unnecessary. |

## Steps

### Step 1 — Dependency, keys, JS

1. `pebble package install @rebble/clay` (needs network; run outside the sandbox). Confirm `package.json` → `"dependencies": {"@rebble/clay": "^1.1.0"}`.
2. `package.json`: `"messageKeys": ["RefreshInterval", "Theme"]`.
3. `src/pkjs/index.js`:
   ```js
   var Clay = require('@rebble/clay');
   var clayConfig = require('./config');
   var clay = new Clay(clayConfig);
   ```
4. `src/pkjs/config.js`:
   ```js
   module.exports = [
     { "type": "heading", "defaultValue": "Eco Stopwatch" },
     { "type": "section", "items": [
       { "type": "heading", "defaultValue": "General" },
       { "type": "select", "messageKey": "RefreshInterval", "label": "Screen Refresh Interval",
         "defaultValue": "10",
         "description": "How often the running timer redraws. Longer intervals save battery. Presses always update the screen immediately.",
         "options": [
           { "label": "1 second",   "value": "1" },  { "label": "2 seconds",  "value": "2" },
           { "label": "5 seconds",  "value": "5" },  { "label": "10 seconds", "value": "10" },
           { "label": "15 seconds", "value": "15" }, { "label": "20 seconds", "value": "20" },
           { "label": "30 seconds", "value": "30" }, { "label": "60 seconds", "value": "60" } ] },
       { "type": "select", "messageKey": "Theme", "label": "Color Theme", "defaultValue": "2",
         "capabilities": ["COLOR"],
         "options": [ { "label": "White on Black", "value": "0" },
                      { "label": "Black on White", "value": "1" },
                      { "label": "Color", "value": "2" } ] },
       { "type": "select", "messageKey": "Theme", "label": "Color Theme", "defaultValue": "0",
         "capabilities": ["BW"],
         "options": [ { "label": "White on Black", "value": "0" },
                      { "label": "Black on White", "value": "1" } ] }
     ]},
     { "type": "submit", "defaultValue": "Save Settings" }
   ];
   ```

**Checkpoint 1:** `pebble build` succeeds for all 7 platforms (if it complains about an unsupported platform, you installed the wrong package).

### Step 2 — Watch side

`settings.h` (final):
```c
void    settings_load(void);                 // persist_read_int(PKEY_INTERVAL/PKEY_THEME) if persist_exists, else defaults; validate
uint8_t settings_interval_s(void);
Theme   settings_theme(void);
void    settings_messaging_init(void);       // app_message_register_inbox_received + app_message_open(128, 64)
```
- Validation: an interval not in `{1,2,5,10,15,20,30,60}` becomes 10. A theme `> 2` becomes the platform default (`PBL_IF_COLOR_ELSE(THEME_COLOR, THEME_WOB)`). On B&W a stored `THEME_COLOR` is kept but rendered as WoB (theme.c already does this).
- `prv_tuple_int(const Tuple *t)`: for `TUPLE_CSTRING`, parse leading decimal digits from `t->value->cstring` (return -1 if there are none). For `TUPLE_INT`/`TUPLE_UINT`, switch on `t->length` (1 → `int8`/`uint8`, 2 → `int16`/`uint16`, 4 → `int32`/`uint32`).
- Inbox handler: `dict_find(iter, MESSAGE_KEY_RefreshInterval)` / `MESSAGE_KEY_Theme` → validate, store, `persist_write_int`, then call `controller_on_settings_changed()` once at the end.
- `main.c`: call `settings_messaging_init()` after the window is pushed.

`controller_on_settings_changed()`:
1. `theme_set(settings_theme()); ui_apply_theme();` (sets the window background and marks all layers dirty);
2. `ui_on_interval_changed();` (recomputes the cached label width);
3. `refresh_reschedule();` (a running timer re-arms immediately on the new cadence, aligned to the new interval's boundary; paused/stopped stays disarmed).

**Checkpoint 2** (emery; `pebble emu-app-config --emulator emery` opens the Clay page in a desktop browser, so **the user** may need to do this part):
- [ ] Theme Black on White: white background, black text, black bar with white icons. Color theme: verbs are red (STOPPED), green (RUNNING), yellow (PAUSED); everything else is white on black.
- [ ] While RUNNING, change the interval 10 → 2: the logs (`ECO_DEBUG_LOG 1`) show `refresh` every 2 s starting immediately, and the label reads `2s`. Change it to 60: one refresh per minute.
- [ ] Exit and relaunch: settings are kept.
- [ ] On aplite, `emu-app-config` shows only two theme options. An injected/stored `2` renders as White on Black.
- [ ] If `emu-app-config` can't be used, the implementer verifies the C path by temporarily calling the inbox logic with a fake value from `main.c`, then removes that code.

### Step 3 — README

Rewrite `README.md` with these sections:
- **What it is**: a battery-first stopwatch.
- **Controls**: the button table from 002/003 (note: while paused with more laps than fit, reset needs ▶ resume first).
- **Settings**: interval options, themes, and that B&W watches have no Color theme.
- **Battery design**: one wakeup per interval, nothing while paused/stopped/covered, no backlight/vibration, state kept across exits via timestamps.
- **Platforms**: the supported list, lap rows visible per platform (from the 002 matrix), and the font sizes.
- **Build**: `pebble build` (if your shell exports host `CFLAGS`, `unset CFLAGS` first); the `@rebble/clay` dependency; emulator commands.
- **Fonts**: Iosevka Custom (which two files are used, and the subsetting via `characterRegex`).
Keep it under ~120 lines.

## Final verification (whole app)

Run this on `emery`, `basalt`, `aplite` (or `diorite`) and `flint` (gating), plus `chalk` and `gabbro` (best-effort), with a screenshot per item where it applies. Afterwards the **user** does a final pass on the Time 2 via `pebble install --cloudpebble`, including the Clay page from the phone app and a real notification overlay.

- [ ] STOPPED → RUNNING → PAUSED → RUNNING → reset, and PAUSED → reset. Icons per the table in each mode.
- [ ] Lap reveal animation, lap overflow, scrolling with ▲/▼ (single and held), no reset while scrolling, the 99-lap cap.
- [ ] ≥100 h shrink: while running, `pebble emu-set-time --emulator <p> <now+360000>` → small font, margins intact. Laps recorded after that also fit.
- [ ] Each theme (on B&W: two themes only).
- [ ] Settings change mid-run re-arms the cadence (logs).
- [ ] Exit/relaunch while running (time kept advancing), while paused, and with laps.
- [ ] Notification overlay: there is no emulator command for it, so verify by code review that `did_focus(false)` cancels the timer (the user checks it on hardware).
- [ ] **Power sanity:** with `ECO_DEBUG_LOG 1`, record `pebble logs` for 2 min each: STOPPED → 0 `refresh` lines; PAUSED → 0; RUNNING at 10 s → 12 ± 1; RUNNING at 60 s → 2 ± 1. Animations: no log activity after the transition ends. `grep -rE "tick_timer_service|light_|vibes_|wakeup_" src/c` returns nothing. Then set `ECO_DEBUG_LOG 0`.
- [ ] `pebble build` is clean, and the memory report shows APLITE heap free > 18000 bytes.

## Done criteria (phase 004 / v1)

All final-verification items pass. `ECO_DEBUG_LOG` is 0. README updated. `git status` shows no build artefacts (`build/`, `node_modules/` ignored). Commit only if the user asks.
