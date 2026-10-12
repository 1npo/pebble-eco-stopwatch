#pragma once
#include <pebble.h>

void icon_play(GContext *ctx, GPoint c, int16_t s);
void icon_pause(GContext *ctx, GPoint c, int16_t s);
void icon_up(GContext *ctx, GPoint c, int16_t s);
void icon_down(GContext *ctx, GPoint c, int16_t s);
void icon_flag(GContext *ctx, GPoint c, int16_t s, GColor bg);
void icon_shield(GContext *ctx, GPoint c, int16_t d);
void icon_cycle(GContext *ctx, GPoint c, int16_t d, int16_t thick, bool clockwise);
void icons_init(void);
void icons_deinit(void);
