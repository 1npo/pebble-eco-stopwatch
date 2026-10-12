#include <pebble.h>
#include "ui.h"
#include "config.h"
#include "anim.h"
#include "controller.h"
#include "format.h"
#include "icons.h"
#include "settings.h"
#include "theme.h"
#include "timer_model.h"

static Window *s_window;
static Layer *s_head, *s_laps, *s_bar;
static GFont s_timer_font, s_small_font, s_text_font;
static int16_t s_label_w;

static void prv_interval_label(char *buf, size_t len) {
  snprintf(buf, len, "%ds", (int)settings_interval_s());
}

static void prv_cache_label_width(void) {
  char label[8];
  prv_interval_label(label, sizeof(label));
  s_label_w = graphics_text_layout_get_content_size(
      label, s_text_font, GRect(0, 0, LINE_W, SUB_BOX_H),
      GTextOverflowModeTrailingEllipsis, GTextAlignmentRight).w;
}

static void prv_head_update(Layer *layer, GContext *ctx) {
  const int16_t w = 2 * CONTENT_CX;
  char text[12];
  uint32_t s = model_elapsed_s();
  format_hms(text, sizeof(text), s);

  bool small = s >= SHRINK_AT_S;
  graphics_context_set_text_color(ctx, theme_fg());
  graphics_draw_text(ctx, text, small ? s_small_font : s_timer_font,
                     GRect(0, small ? TIMER_SMALL_DY : 0, w, TIMER_BOX_H),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  TimerState st = model_state();
  const char *verb = st == TS_RUNNING ? "RUNNING" : st == TS_PAUSED ? "PAUSED" : "STOPPED";
  GRect line = GRect(CONTENT_CX - LINE_W / 2, SUB_Y, LINE_W, SUB_BOX_H);
  graphics_context_set_text_color(ctx, theme_verb(st));
  graphics_draw_text(ctx, verb, s_text_font, line, GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentLeft, NULL);

  char label[8];
  prv_interval_label(label, sizeof(label));
  graphics_context_set_text_color(ctx, theme_fg());
  graphics_draw_text(ctx, label, s_text_font, line, GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentRight, NULL);

  graphics_context_set_fill_color(ctx, theme_fg());
  GPoint c = GPoint(line.origin.x + LINE_W - s_label_w - 3 - REFRESH_ICON_D / 2,
                    SUB_Y + CAP_TOP + REFRESH_ICON_D / 2);
  icon_cycle(ctx, c, REFRESH_ICON_D, REFRESH_THICK, true);
}

static void prv_laps_update(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, theme_fg());
  graphics_fill_rect(ctx, GRect(CONTENT_CX - LINE_W / 2, 0, LINE_W, RULE_THICK), 0, GCornerNone);

  uint8_t count = model_lap_count();
  uint8_t first = controller_scroll_first();
  uint8_t rows = count - first;
  if (rows > LAP_ROWS_VISIBLE) rows = LAP_ROWS_VISIBLE;
  graphics_context_set_text_color(ctx, theme_fg());
  for (uint8_t i = 0; i < rows; i++) {
    char hms[12];
    char text[24];
    format_hms(hms, sizeof(hms), model_lap_ms(first + i) / 1000);
    snprintf(text, sizeof(text), "LAP %02d  %s", (int)(first + i + 1), hms);
    graphics_draw_text(ctx, text, s_text_font,
                       GRect(0, ROW0_Y + i * ROW_PITCH, 2 * CONTENT_CX, ROW_PITCH + 4),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  }
}

static void prv_bar_update(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, theme_bar_bg());
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, theme_bar_fg());
  graphics_context_set_stroke_color(ctx, theme_bar_fg());

  const int16_t ox = SCREEN_W - BAR_W;
  GPoint up = GPoint(BAR_ICON_X - ox, BAR_ICON_Y_UP);
  GPoint sel = GPoint(BAR_SELECT_X - ox, BAR_ICON_Y_SEL);
  GPoint down = GPoint(BAR_ICON_X - ox, BAR_ICON_Y_DOWN);

  switch (controller_mode()) {
    case UI_STOPPED:
      icon_play(ctx, sel, ICON_SZ);
      break;
    case UI_RUNNING:
      if (model_lap_count() < MAX_LAPS) icon_flag(ctx, up, ICON_SZ, theme_bar_bg());
      icon_pause(ctx, sel, ICON_SZ);
      icon_cycle(ctx, down, ICON_SZ, RESET_THICK, false);
      break;
    case UI_PAUSED:
      icon_play(ctx, sel, ICON_SZ);
      icon_cycle(ctx, down, ICON_SZ, RESET_THICK, false);
      break;
    case UI_PAUSED_SCROLL:
      icon_up(ctx, up, ICON_SZ);
      icon_play(ctx, sel, ICON_SZ);
      icon_down(ctx, down, ICON_SZ);
      break;
  }
}

void ui_set_laps_layout(bool shown) {
  GRect h = layer_get_frame(s_head);
  h.origin.y = shown ? HEAD_Y_LAPS : HEAD_Y_CENTER;
  layer_set_frame(s_head, h);
  GRect l = layer_get_frame(s_laps);
  l.origin.y = shown ? RULE_Y : SCREEN_H;
  layer_set_frame(s_laps, l);
  layer_set_hidden(s_laps, !shown);
}

void ui_create(Window *w) {
  s_window = w;
  s_timer_font = fonts_load_custom_font(resource_get_handle(RES_FONT_TIMER));
  s_small_font = fonts_load_custom_font(resource_get_handle(RES_FONT_TIMER_SMALL));
  s_text_font = fonts_load_custom_font(resource_get_handle(RES_FONT_TEXT));
  prv_cache_label_width();

  Layer *root = window_get_root_layer(w);
  s_head = layer_create(GRect(0, HEAD_Y_CENTER, 2 * CONTENT_CX, HEAD_H));
  layer_set_update_proc(s_head, prv_head_update);
  layer_add_child(root, s_head);
  s_laps = layer_create(GRect(0, RULE_Y, 2 * CONTENT_CX, SCREEN_H - RULE_Y));
  layer_set_update_proc(s_laps, prv_laps_update);
  layer_add_child(root, s_laps);
  ui_set_laps_layout(model_lap_count() > 0);
  s_bar = layer_create(GRect(SCREEN_W - BAR_W, 0, BAR_W, SCREEN_H));
  layer_set_update_proc(s_bar, prv_bar_update);
  layer_add_child(root, s_bar);
}

void ui_destroy(void) {
  anim_cancel();
  layer_destroy(s_head);
  layer_destroy(s_laps);
  layer_destroy(s_bar);
  fonts_unload_custom_font(s_timer_font);
  fonts_unload_custom_font(s_small_font);
  fonts_unload_custom_font(s_text_font);
}

void ui_mark_dirty(void) {
  layer_mark_dirty(s_head);
  layer_mark_dirty(s_laps);
}

void ui_mark_laps_dirty(void) {
  layer_mark_dirty(s_laps);
}

void ui_apply_theme(void) {
  window_set_background_color(s_window, theme_bg());
  layer_mark_dirty(s_head);
  layer_mark_dirty(s_laps);
  layer_mark_dirty(s_bar);
}

void ui_on_interval_changed(void) {
  prv_cache_label_width();
  layer_mark_dirty(s_head);
}

Layer *ui_head_layer(void) { return s_head; }
Layer *ui_laps_layer(void) { return s_laps; }
Layer *ui_bar_layer(void) { return s_bar; }
