# Store listing draft: Eco Stopwatch

**Title:** Eco Stopwatch
**Category:** Tools & Utilities
**Source:** https://github.com/1npo/pebble-eco-stopwatch
**Version:** 1.0.0

## Short pitch (one line)

A battery-first stopwatch: the screen redraws only every few seconds, not every second.

## Description

Eco Stopwatch is a stopwatch built to be left running for hours without draining your watch. Instead of redrawing every second, it updates the screen once every N seconds (10 by default, adjustable from 1 to 60) and once immediately on every button press. There is no tick service, no backlight use and no background wakeups. Elapsed time comes from wall-clock timestamps rather than counted ticks, so it stays exact at any refresh interval and keeps counting while the app is closed.

**Features**
- Big stacked hours / minutes / seconds display, with up to 99 laps (cumulative time at each press)
- Scroll through laps while paused; the timer switches to a smaller font past 100 hours (cap 999:59:59)
- Icon bar on the right always shows what the three buttons do
- State (running, paused, laps) survives exiting the app
- Safeguard: optionally halt a running timer at a set limit (5 minutes up to 24 hours) so a forgotten timer doesn't run forever. The watch buzzes once and shows HALTED; reset to clear
- Themes: White on Black, Black on White, and a Color theme with a colored state label (color watches only)
- Settings from the Pebble phone app, applied instantly, stored on the watch
- Works on all Pebble models: Pebble, Pebble Steel, Pebble Time, Time Steel, Time Round, Pebble 2, Pebble Time 2 and the newer Core Devices watches

**Controls**

| Mode | Up | Select | Down |
|---|---|---|---|
| Stopped | n/a | Start | n/a |
| Running | Lap | Pause | Reset |
| Paused | n/a | Resume | Reset |
| Paused, many laps | Scroll up | Resume | Scroll down |

**Credits:** Fonts are custom builds of Iosevka by Renzhi Li (SIL Open Font License 1.1).

## Release notes (1.0.0)

First release.

## Screenshots to upload

Files are in `docs/screenshots/`:
- aplite: `aplite-stopped.png`, `aplite-white-on-black.png`, `aplite-black-on-white.png` (reuse for basalt if you prefer)
- basalt / chalk / diorite / flint / gabbro: `<platform>-running-laps.png`
- emery: `emery-running-laps.png`, `emery-stopped.png`, `emery-paused-scroll.png`, `emery-halted.png`, `emery-black-on-white.png`, `emery-white-on-black.png`

## Icons

`docs/store/icon-large-144.png`, `docs/store/icon-small-48.png`
