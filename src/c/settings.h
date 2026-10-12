#pragma once
#include <pebble.h>
#include "theme.h"

void    settings_load(void);
uint8_t settings_interval_s(void);
Theme   settings_theme(void);
