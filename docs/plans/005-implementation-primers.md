# 005 — Implementation primers (Sonnet 5.5, Medium effort)

There is one primer per phase. Start a **fresh** session in `~/dev/projects/pebble-eco-stopwatch` for each phase, and paste everything between that phase's `---` lines. Run the phases in order, and only start the next one after you've checked the previous phase on your Time 2.

---

## Phase 1 — Core stopwatch (plan 002)

---

You are implementing **Eco Stopwatch**, a battery-optimized stopwatch watchapp for Pebble, written in C with the Pebble SDK. Your job in this session is **phase 1 only**, as specified in `docs/plans/002-core-stopwatch.md`.

**Read first, in this order:**
1. `docs/plans/002-core-stopwatch.md`, the full plan for this phase. It is also the reference document (platform matrix, layout constants, module layout, persist keys) for later phases.
2. `docs/plans/000-initial-request.md`, the original spec, for context. Where 002 resolves something differently (for example the `10s` label, or the name "Eco Stopwatch" instead of "Eco Timer"), **002 wins**.
3. The current `package.json`, `wscript`, and `src/c/`.

Skim `003-laps-and-animation.md` and `004-settings-and-themes.md` only so you understand what later phases will build on. **Do not implement anything from them.** The exception is anything 002 explicitly tells you to define now (for example all persist keys, and the `UI_PAUSED_SCROLL` enum value).

**Rules:**
- Follow the plan's steps in order. Each step ends in a checkpoint; don't start the next step until that checkpoint passes. If a checkpoint fails and the fix requires departing from the plan, stop and ask me. Don't redesign.
- Every decision in the plan's "Decisions & assumptions" table is final. Use the constants, struct layouts, resource names, file names and API calls exactly as written.
- **Build command:** always run `unset CFLAGS && pebble build` as a single command. My shell exports host `CFLAGS` that break the ARM compiler, and each of your Bash calls starts a fresh shell.
- **Emulator commands** (`pebble install --emulator …`, `pebble screenshot …`, `pebble logs …`, `pebble emu-*`, `pebble kill`) must run with the sandbox disabled, because the sandbox blocks the emulator's localhost connection. If one fails with `libpebble2.exceptions.TimeoutError`, re-run it once before debugging; the first boot is slow.
- Look at every screenshot yourself with the Read tool and check it against the acceptance rules. Don't assume it's fine.
- Emery (my Pebble Time 2), basalt and aplite are the gating platforms. Chalk and gabbro must build and run, but their screenshots are best-effort.
- **Never** run `pebble install --cloudpebble`. I test on the real watch myself.
- Battery is the product. Never add `tick_timer_service`, `light_*`, `vibes_*` or `wakeup_*` calls, extra timers, or redraws the plan doesn't call for.
- Remove all temporary/debug code the plan asks for once its checkpoint passes. Leave `ECO_DEBUG_LOG` at `0`.
- Stay on the current git branch. Don't create branches, commit or push unless I ask.

**When finished:** report each checkpoint and done criterion as pass/fail, with evidence (log excerpts, which screenshots you checked). List anything you changed from the plan and why, and any open issues. Then tell me the build is ready for me to install with `pebble install --cloudpebble`.

---

## Phase 2 — Laps and animation (plan 003)

---

You are continuing **Eco Stopwatch**, a battery-optimized Pebble stopwatch in C. Phase 1 (`docs/plans/002-core-stopwatch.md`) is complete and checked on hardware. Your job in this session is **phase 2 only**, as specified in `docs/plans/003-laps-and-animation.md`.

**Read first, in this order:**
1. `docs/plans/003-laps-and-animation.md`, the plan for this phase.
2. `docs/plans/002-core-stopwatch.md`, the reference for the platform matrix, layout constants (`HEAD_Y_LAPS`, `RULE_Y`, `ROW0_Y`, `ROW_PITCH`, `LAP_ROWS_VISIBLE`, …), persist keys, tooling notes and module layout. Don't redo any of its steps.
3. All of `src/c/` as it stands now, so your changes fit the existing code and style.

Don't implement anything from `004-settings-and-themes.md`.

**Things that are easy to get wrong in this phase. The plan explains each one; follow it exactly:**
- Set animation duration and curve on **each child** `PropertyAnimation`, **never** on the spawn. This was tested: values set on the spawn are ignored.
- Never call `animation_destroy`/`property_animation_destroy`. Finished animations free themselves (tested). Clear `s_anim` in the `.stopped` handler.
- **There's no reset while paused with more laps than fit** (`UI_PAUSED_SCROLL`): only ▲ ▶ ▼, and no long-press actions anywhere. This is my decision. Don't add one.
- UP/DOWN repeat only in `UI_PAUSED_SCROLL`. Everywhere else they are single clicks, so holding 🏁 can't record a burst of laps.
- Only the first lap animates. Later laps and scrolling redraw instantly.

**Rules:**
- Follow the plan's steps in order, each ending in its checkpoint. If a checkpoint fails and the fix requires departing from the plan, stop and ask me. Don't redesign.
- **Build command:** always run `unset CFLAGS && pebble build` as a single command, because my shell exports host `CFLAGS` that break the ARM compiler.
- **Emulator commands** (`pebble install --emulator …`, `pebble screenshot …`, `pebble logs …`, `pebble emu-*`, `pebble kill`) must run with the sandbox disabled. On `libpebble2.exceptions.TimeoutError`, re-run once.
- Check every screenshot yourself with the Read tool. To check the animation, take screenshots during the transition and after it ends.
- Emery, basalt and aplite are gating platforms. Chalk and gabbro are best-effort.
- **Never** run `pebble install --cloudpebble`; I do that.
- Battery is the product. After an animation ends, nothing may keep running (verify with `ECO_DEBUG_LOG 1`, then set it back to `0`).
- Remove all temporary/debug code once its checkpoint passes.
- Stay on the current git branch. Don't create branches, commit or push unless I ask.

**When finished:** report each checkpoint and done criterion as pass/fail, with evidence. Include the heap check from risk A1. List any departures from the plan and any open issues. Then tell me the build is ready for `pebble install --cloudpebble`.

---

## Phase 3 — Settings, themes, README (plan 004)

---

You are finishing **Eco Stopwatch**, a battery-optimized Pebble stopwatch in C. Phases 1 and 2 (`docs/plans/002-core-stopwatch.md`, `docs/plans/003-laps-and-animation.md`) are complete and checked on hardware. Your job in this session is **phase 3**, as specified in `docs/plans/004-settings-and-themes.md`, including its "Final verification" section.

**Read first, in this order:**
1. `docs/plans/004-settings-and-themes.md`, the plan for this phase.
2. `docs/plans/002-core-stopwatch.md` and `docs/plans/003-laps-and-animation.md`, as reference: constants, persist keys (`PKEY_INTERVAL`, `PKEY_THEME`), button tables, tooling notes. Don't redo their steps.
3. All of `src/c/`, `package.json`, and `README.md` as they stand now.

**Things that are easy to get wrong in this phase:**
- Use **`@rebble/clay`**. Install it with `pebble package install @rebble/clay` (it needs network, so run it with the sandbox disabled). **Not** `pebble-clay`, which refuses to build for flint and gabbro.
- The theme setting is **two** `select` items that share `messageKey: "Theme"`, gated by `capabilities: ["COLOR"]` and `["BW"]`. Use the config from the plan verbatim.
- Clay `select` values reach the watch as **C strings**. The watch must parse `TUPLE_CSTRING` as well as int tuples. `atoi` isn't available; write the small digit parser from the plan.
- When settings change on a running timer, the refresh timer must re-arm on the new interval immediately (`controller_on_settings_changed`).
- The interval label stays `"%ds"` (lowercase `s`).
- The README must not mention long-press reset. It doesn't exist; reset while scrolling laps means resume first.

**Rules:**
- Follow the plan's steps in order, each ending in its checkpoint. If something requires departing from the plan, stop and ask me.
- **Build command:** always run `unset CFLAGS && pebble build` as a single command.
- **Emulator commands** must run with the sandbox disabled; on `libpebble2.exceptions.TimeoutError`, re-run once. `pebble emu-app-config` opens a desktop browser. If you can't complete the Clay page that way, use the fallback in Checkpoint 2 (a temporary injected value) and tell me which items I need to check by hand.
- Check every screenshot yourself with the Read tool. Emery, basalt, aplite and flint are gating platforms. Chalk and gabbro are best-effort.
- **Never** run `pebble install --cloudpebble`; I do that.
- Battery is the product. The power sanity check in "Final verification" must pass. End with `ECO_DEBUG_LOG` at `0` and no temporary code left.
- Stay on the current git branch. Don't create branches, commit or push unless I ask.

**When finished:** report every "Final verification" item and done criterion as pass/fail, with evidence (including the refresh counts from the power check). List any departures from the plan, open issues, and the items I need to check on the Time 2: the Clay page from the phone app, a real notification overlay, and how the subtext font looks. Then tell me the build is ready for `pebble install --cloudpebble`.

---
