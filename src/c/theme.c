#include <pebble.h>
#include "theme.h"

static Theme s_theme = PBL_IF_COLOR_ELSE(THEME_COLOR, THEME_WOB);

void theme_set(Theme t) {
  s_theme = t;
}

GColor theme_bg(void) {
  return s_theme == THEME_BOW ? GColorWhite : GColorBlack;
}

GColor theme_fg(void) {
  return s_theme == THEME_BOW ? GColorBlack : GColorWhite;
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
      case TS_PAUSED:  return GColorYellow;
      case TS_RUNNING: return GColorGreen;
    }
  }
#endif
  return theme_fg();
}
