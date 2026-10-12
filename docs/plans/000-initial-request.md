I want to create a timer watchapp for the Pebble that's optimized for low battery usage. My requirements are outlined below. In a new session, I will have Opus 5.5 write the plan for the first version of this app. Then I'll use Sonnet 5.5 Medium to implement the plan that Opus created. Write a prompt I can use to prime Opus to create the plan.

# Features

- The screen is refreshed at a custom interval (default 10 seconds)
- Timer displays hours, minutes, and seconds
- Beneath the timer:
  - A verb indicating the state of the timer:
    - Stopped
    - Paused
    - Running
  - The screen refresh interval (eg, 10 sec)
- On the right border of the screen, an icon bar of standard width containing these button assignments:
  - When stopped (new timer at 00:00:00):
    - Button 2: Play icon (“▶”)
  - When running:
    - Button 1: Checkered flag icon ("🏁")
    - Button 2: Pause icon ("⏸")
    - Button 3: Reset icon ("⟲")
  - When paused:
    - Button 2: Play icon (“▶”)
    - Button 3: Reset icon ("⟲")
  - When paused with more laps tracked than can be displayed on the screen:
    - Button 1: Up triangle ("▲")
    - Button 2: Play icon (“▶”)
    - Button 3: Down triangle ("▼”)

# Design Details

- Timer is centered on the screen
- Status verb and refresh interval are both on the same line beneath the timer
  - Status verb is left aligned
  - Refresh interval is right aligned
- Timer font is IosevkaCustomForPebble Heavy
  - Large enough to fill the display with a clean/healthy margin between the rightmost digit and the icon bar
  - Size shrinks if the timer exceeds 99:99:99
- Text beneath the timer font is IosevkaCustomForPebbleNarrow Regular
  - Smaller than the timer font, but big enough to be legible
  - Capitalized
- When the colored theme is picked, timer state verbs have these colors:
  - Stopped: Red
  - Paused: Yellow
  - Running: Green
- Screen refresh interval should include an appropriate icon to the left (eg, <refresh-interval-icon> 10 sec)
- When a lap is set:
  - The timer and subtext is shifted up so its aligned to the top
  - A horizontal line appears beneath the subtext
  - The text "LAP 01  <the time when lap button pressed>" appears beneath it
  - If more laps are set than can be displayed, the earlier laps disappear to make room for the newer laps displayed at the bottom
  - The shifting of the timer and subtext, and appearance of the horizontal line and first lap text, should be smooth like the smooth transition of card animations on standard Pebble apps

# Settings (Clay)

- General
  - Screen Refresh Interval (default 10 sec)
  - Color Theme
    - White on Black
    - Black on White
    - Color (White on Black with colored timer state verb)

# Artifacts

- All plan documents (the plan Opus creates, and any subsequent plans created during implementation, if a phased approach is required), go in `docs/plans` and follow the numbering convention currently in place (start with `000-` and increment by 1 as each new plan is created)

# Comments

- I have a Pebble Time 2, but this app should work on all Pebble watches if possible
