#pragma once
#include <pebble.h>
#include "theme.h"

typedef enum { FONT_IOSEVKA = 0, FONT_SYSTEM = 1 } FontChoice;

void settings_load(void);
uint8_t settings_interval_s(void);
Theme settings_theme(void);
uint16_t settings_safeguard_min(void); // 0 = off
FontChoice settings_font(void);
void settings_messaging_init(void);
