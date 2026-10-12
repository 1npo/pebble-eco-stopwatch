#pragma once
#include <pebble.h>

typedef enum { UI_STOPPED, UI_RUNNING, UI_PAUSED, UI_PAUSED_SCROLL } UiMode;

UiMode  controller_mode(void);
uint8_t controller_scroll_first(void);
void   controller_init(Window *w);
void   controller_on_focus(bool in_focus);
void   controller_on_settings_changed(void);
bool   controller_safeguard_check(void);
