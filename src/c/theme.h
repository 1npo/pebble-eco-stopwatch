#pragma once
#include <pebble.h>
#include "timer_model.h"

typedef enum { THEME_WOB = 0, THEME_BOW = 1, THEME_COLOR = 2 } Theme;

void theme_set(Theme t);
GColor theme_bg(void);
GColor theme_fg(void);
GColor theme_bar_bg(void);
GColor theme_bar_fg(void);
GColor theme_verb(TimerState s);
