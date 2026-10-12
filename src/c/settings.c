#include <pebble.h>
#include "settings.h"

static uint8_t s_interval_s = 10;
static Theme s_theme = PBL_IF_COLOR_ELSE(THEME_COLOR, THEME_WOB);

void settings_load(void) {
  s_interval_s = 10;
  s_theme = PBL_IF_COLOR_ELSE(THEME_COLOR, THEME_WOB);
}

uint8_t settings_interval_s(void) {
  return s_interval_s;
}

Theme settings_theme(void) {
  return s_theme;
}
