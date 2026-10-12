#include <pebble.h>
#include "theme.h"

static Theme s_theme = PBL_IF_COLOR_ELSE(THEME_COLOR, THEME_WOB);

void theme_set(Theme t) {
  s_theme = t;
}

// Black on White, and Color (which only differs by its verb colours). On B&W
// watches a stored Color renders as White on Black.
static bool prv_light(void) {
  return s_theme == THEME_BOW || (s_theme == THEME_COLOR && PBL_IF_COLOR_ELSE(true, false));
}

GColor theme_bg(void) {
  return prv_light() ? GColorWhite : GColorBlack;
}

GColor theme_fg(void) {
  return prv_light() ? GColorBlack : GColorWhite;
}

GColor theme_bar_bg(void) {
  return theme_fg();
}

GColor theme_bar_fg(void) {
  return theme_bg();
}

GColor theme_verb(TimerState s) {
#if defined(PBL_COLOR)
  if (s_theme == THEME_COLOR) {
    switch (s) {
      case TS_STOPPED: return GColorRed;
      case TS_PAUSED:  return GColorLimerick;  // plain yellow is unreadable on white
      case TS_RUNNING: return GColorDarkGreen;
      case TS_TERMINATED: return GColorOrange;
    }
  }
#endif
  return theme_fg();
}
