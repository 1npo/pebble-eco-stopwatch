# 003 — Laps, lap list, reveal animation

Phase 2 of 3. **Prerequisite:** `002-core-stopwatch.md` is done (all of its done criteria pass). The platform matrix, constants (`HEAD_Y_LAPS`, `RULE_Y`, `ROW0_Y`, `ROW_PITCH`, `LAP_ROWS_VISIBLE`, …), tooling notes and persist keys are defined in 002. Use them as written there.

## Goal

Button 1 records laps while running. The first lap slides the timer block to the top while the rule and lap list slide up from the bottom. The list shows the newest lap at the bottom. While paused with more laps than fit, ▲/▼ scroll the list (and reset is unavailable in that mode). Laps survive app exit. Reset clears them and reverses the animation.

## Decisions & assumptions

| # | Decision | Why |
|---|----------|-----|
| L1 | A lap stores **cumulative elapsed ms** at the moment of the press (`uint32_t`, so up to 1193 h, more than the 999 h cap). It is displayed as `LAP NN  hh:mm:ss` (two spaces, cumulative time, seconds truncated). | The spec says "the time when lap button pressed". ms keeps the option of split deltas later at no cost. |
| L2 | `MAX_LAPS = 99`. At 99, the 🏁 icon is hidden and UP is ignored while running. | The `LAP NN` format is two digits. 99 × 4 B = 396 B of RAM is trivial even on aplite. |
| L3 | Lap rows are drawn by **one custom `Layer`** (`laps_layer`), not `MenuLayer`/`ScrollLayer`. | At most 7 rows of text, no selection, no inertia. The custom layer costs no extra allocations, needs no extra click handling, and redraws only on change. `ScrollLayer` would hijack UP/DOWN and animate scrolling (wasted frames). |
| L4 | Rows are a single string `"LAP %02d  %s"`, centred on `CONTENT_CX`, in the text font and `theme_fg()`. | Digits in Narrow are tabular (all 0.5 em, measured), so rows align as columns. |
| L5 | Visible window: `first = max(0, count - LAP_ROWS_VISIBLE)` (newest at the bottom, oldest drop off the top) in every mode **except** `UI_PAUSED_SCROLL`, which uses `s_scroll_first`. Rows fill from the top: with fewer laps than rows, the list is top-aligned under the rule. | Matches the spec. |
| L6 | Overflow = `lap_count > LAP_ROWS_VISIBLE`. On entering PAUSED with overflow, `s_scroll_first = count - LAP_ROWS_VISIBLE` (the newest are visible). ▲ = `first - 1` (min 0), ▼ = `first + 1` (max `count - LAP_ROWS_VISIBLE`). At a limit the press does nothing. Both arrows stay visible (as the spec says). | Line-by-line scrolling is predictable. |
| L7 | In `UI_PAUSED_SCROLL`, UP/DOWN use `window_single_repeating_click_subscribe(..., 150, ...)` so holding scrolls. In every other mode they are single clicks, so holding 🏁 cannot record a burst of laps. The click config is re-applied on every mode change via `window_set_click_config_provider(window, prv_click_config)`. | — |
| L8 | **No reset in `UI_PAUSED_SCROLL`, and no long-press actions anywhere.** To reset with many laps: ▶ resume, then ⟲. | User decision: a long-press reset is acceptable only with a clean on-screen hint, and there isn't room for one in the 30–40 px bar (a mini ⟲ badge next to ▶ reads as "replay"). Revisit later. |
| L9 | Animation: `property_animation_create_layer_frame` on `head_layer` and `laps_layer`, combined with `animation_spawn_create`, **300 ms** reveal / **250 ms** hide, `AnimationCurveEaseInOut`. **Duration and curve are set on each child animation, never on the spawn** (A1). `from` = `NULL` (current frame), so an interrupted animation reverses smoothly from wherever it is. | Same primitive, curve and duration range as system card transitions. About 9 frames per transition. Nothing keeps running afterwards. |
| L10 | The rule and the first lap **slide** in (the laps layer moves from `y = SCREEN_H` to `y = RULE_Y`). They do not fade. | Pebble has no alpha compositing, and fades are impossible on B&W. A slide matches the card-animation feel on every platform. |
| L11 | Later laps (2+) appear instantly; no per-lap animation. | The spec asks for smoothness only on the reveal, and it saves frames. |
| L12 | When laps are restored at launch, the lap layout is applied **without** animation. | — |

## Open risks

- **A1 Animation behaviour (VERIFIED in the emulator, SDK 4.33.1, emery + aplite, 2026-10-11).**
  - *Lifetime:* a spawn of two `PropertyAnimation`s, scheduled 12 times with no manual destroy, left `heap_bytes_free()` flat (emery 129140, aplite 21972 every cycle). So finished animations, including the spawn's children, are freed automatically. Rule: keep one `static Animation *s_anim`, set it to `NULL` in its `.stopped` handler, and **never** call `animation_destroy`/`property_animation_destroy`.
  - *Duration/curve on a spawn:* `animation_set_duration(spawn, 1000)` did **not** propagate. The children finished at ~270 ms (the 250 ms default). Setting duration and curve **on each child** gave ~1000 ms and the ease-in-out shape (11 % travelled at 25 % time). The spawn's `.stopped` fires after both children.
  - *Not tested:* interrupting with `animation_unschedule`. Per `pebble.h` (the `animation_schedule` note), unscheduling calls `.stopped` with `finished = false`; the checkpoint covers it.
- **A2 Round screens show fewer rows** (chalk 4, gabbro 6) because the bezel narrows. This is intentional and documented in the README (004).

## Changes

### Step 1 — Model + persistence

`timer_model.h` additions:
```c
uint8_t  model_lap_count(void);
uint32_t model_lap_ms(uint8_t index);        // 0-based, oldest first
bool     model_add_lap(void);                // RUNNING && count < MAX_LAPS: store (uint32_t)model_elapsed_ms(); return true if stored
// model_reset() also sets lap count to 0
void     model_restore_laps(const uint32_t *laps, uint8_t count);
```
Storage: `static uint32_t s_laps[MAX_LAPS]; static uint8_t s_lap_count;`.

`persist.c`:
- Save: `PersistTimer.lap_count = count`. If `count > 0`, write `PKEY_LAPS_A` with `min(count, 50) * 4` bytes, else `persist_delete(PKEY_LAPS_A)`. If `count > 50`, write `PKEY_LAPS_B` with `(count - 50) * 4` bytes, else `persist_delete(PKEY_LAPS_B)`. (`PERSIST_DATA_MAX_LENGTH` is 256, so 50 × 4 = 200 fits.)
- Load: read both chunks per `lap_count` (clamp to `MAX_LAPS`). If a chunk read returns fewer bytes than expected, set the lap count to what was actually read.

**Checkpoint 1:** builds. Temporary code that adds 3 laps and exits/relaunches restores 3 laps (log them). Remove the temporary code.

### Step 2 — Laps layer + static layout

`ui.c`:
- Create `laps_layer` (added between head and bar) with frame `GRect(0, RULE_Y, 2*CONTENT_CX, SCREEN_H - RULE_Y)`, hidden when `lap_count == 0`.
- `ui_set_laps_layout(bool shown)`, no animation: shown → head y = `HEAD_Y_LAPS`, laps y = `RULE_Y`, laps not hidden; not shown → head y = `HEAD_Y_CENTER`, laps y = `SCREEN_H`, laps hidden.
- In `ui_create`, call `ui_set_laps_layout(model_lap_count() > 0)` (L12).
- Laps update proc:
  1. rule: `graphics_fill_rect(ctx, GRect(CONTENT_CX - LINE_W/2, 0, LINE_W, RULE_THICK), 0, GCornerNone)` in `theme_fg()`;
  2. compute `first` per L5 (ask `controller_scroll_first()`);
  3. for `i` in `0 .. min(LAP_ROWS_VISIBLE, count - first) - 1`, draw lap `first + i` with `format_hms(lap_ms / 1000)` into `"LAP %02d  %s"` (lap number = `first + i + 1`) in `GRect(0, ROW0_Y + i*ROW_PITCH, 2*CONTENT_CX, ROW_PITCH + 4)`, `GTextAlignmentCenter`, `GTextOverflowModeTrailingEllipsis`.
- `ui_mark_dirty()` now also marks `laps_layer`.

### Step 3 — Controller: lap, scroll

- `controller_mode()` returns `UI_PAUSED_SCROLL` when PAUSED and `lap_count > LAP_ROWS_VISIBLE`.
- UP in UI_RUNNING: `if (model_add_lap()) { if (model_lap_count() == 1) anim_show_laps(); redraw }`.
- On entering PAUSED (from pause): `s_scroll_first = max(0, count - LAP_ROWS_VISIBLE)`.
- UP/DOWN in UI_PAUSED_SCROLL: adjust `s_scroll_first` per L6. If it changed, mark only `laps_layer` dirty.
- SELECT in any paused mode: resume (and the scroll window naturally returns to the newest rows via L5).
- DOWN reset (UI_RUNNING/UI_PAUSED only): `bool had = model_lap_count() > 0; model_reset(); if (had) anim_hide_laps();` then the standard transition steps from 002.
- Bar icons: add ▲/▼ for UI_PAUSED_SCROLL. In UI_RUNNING, hide 🏁 when `lap_count == MAX_LAPS`.
- After any transition that changes the mode, call `window_set_click_config_provider(s_window, prv_click_config)` (L7). Subscriptions:
  - `UI_PAUSED_SCROLL`: repeating UP/DOWN at 150 ms, single SELECT;
  - all other modes: single clicks on UP/SELECT/DOWN.

### Step 4 — Animation (`anim.c/.h`)

```c
void anim_show_laps(void);   // 300 ms
void anim_hide_laps(void);   // 250 ms
void anim_cancel(void);      // called from ui_destroy
```
Implementation:
```c
static Animation *s_anim; static bool s_target_shown;
static void prv_stopped(Animation *a, bool finished, void *ctx) {
  s_anim = NULL;
  if (finished && !s_target_shown) layer_set_hidden(laps_layer, true);
}
static void prv_run(bool show, uint32_t ms) {
  if (s_anim) animation_unschedule(s_anim);          // fires prv_stopped(finished=false)
  s_target_shown = show;
  if (show) layer_set_hidden(laps_layer, false);
  GRect head_to = layer_get_frame(head); head_to.origin.y = show ? HEAD_Y_LAPS : HEAD_Y_CENTER;
  GRect laps_to = layer_get_frame(laps); laps_to.origin.y = show ? RULE_Y : SCREEN_H;
  Animation *a = property_animation_get_animation(property_animation_create_layer_frame(head, NULL, &head_to));
  Animation *b = property_animation_get_animation(property_animation_create_layer_frame(laps, NULL, &laps_to));
  animation_set_duration(a, ms); animation_set_curve(a, AnimationCurveEaseInOut);   // on the CHILDREN (A1)
  animation_set_duration(b, ms); animation_set_curve(b, AnimationCurveEaseInOut);
  s_anim = animation_spawn_create(a, b, NULL);       // no duration/curve on the spawn
  animation_set_handlers(s_anim, (AnimationHandlers){ .stopped = prv_stopped }, NULL);
  animation_schedule(s_anim);
}
```

**Checkpoint (emery and aplite gating; chalk best-effort)**, using screenshots mid-way and at the end:
- [ ] Start, then UP: the head glides to the top while the rule and `LAP 01  00:00:0X` rise from the bottom. Final positions match the constants and the timer shows the same value as LAP 01.
- [ ] Press UP 8 more times: newest at the bottom, oldest drop off. Row counts: emery 7, chalk 4, aplite 6.
- [ ] Pause: ▲ ▶ ▼ shown, no ⟲. ▲ ×3 shows older laps; holding ▼ (`emu-button push down`, wait 1 s, `release down`) scrolls back to the newest and stops at the end.
- [ ] Holding SELECT in PAUSED_SCROLL only resumes (no reset). Then DOWN (⟲ in RUNNING) resets with the reverse animation, back to centred `00:00:00 STOPPED`.
- [ ] Paused with ≤ `LAP_ROWS_VISIBLE` laps: ⟲ is shown and resets with the reverse animation.
- [ ] Reset during the reveal animation (press DOWN within 300 ms of the first lap): it smoothly reverses, with no stuck or hidden layer.
- [ ] Exit with 9 laps while paused, then relaunch: laps restored, no animation, scroll mode active.
- [ ] Record 99 laps (`pebble emu-button --emulator emery click up --repeat 99 --interval 50`): the 🏁 icon disappears and further UP does nothing.
- [ ] After a reveal ends, `ECO_LOG` shows no further callbacks (no stray timers). A1 heap check passes.

## Done criteria (phase 003)

All checkpoint items pass on emery and aplite. A basalt screenshot with overflow has nothing clipped. Round screens are best-effort. Build is clean on all 7 platforms. Tell the user the build is ready for `pebble install --cloudpebble`.
