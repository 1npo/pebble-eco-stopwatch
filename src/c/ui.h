#pragma once
#include <pebble.h>

void   ui_create(Window *w);
void   ui_destroy(void);
void   ui_mark_dirty(void);
void   ui_mark_laps_dirty(void);
void   ui_set_laps_layout(bool shown);
void   ui_apply_theme(void);
void   ui_on_interval_changed(void);
Layer *ui_head_layer(void);
Layer *ui_laps_layer(void);
Layer *ui_bar_layer(void);
