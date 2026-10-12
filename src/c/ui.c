#include <pebble.h>
#include "ui.h"
#include "config.h"
#include "controller.h"
#include "format.h"
#include "icons.h"
#include "settings.h"
#include "theme.h"
#include "timer_model.h"

static Window *s_window;
static Layer *s_head, *s_bar;
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
      icon_flag(ctx, up, ICON_SZ, theme_bar_bg());
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
  s_bar = layer_create(GRect(SCREEN_W - BAR_W, 0, BAR_W, SCREEN_H));
  layer_set_update_proc(s_bar, prv_bar_update);
  layer_add_child(root, s_bar);
}

void ui_destroy(void) {
  layer_destroy(s_head);
  layer_destroy(s_bar);
  fonts_unload_custom_font(s_timer_font);
  fonts_unload_custom_font(s_small_font);
  fonts_unload_custom_font(s_text_font);
}

void ui_mark_dirty(void) {
  layer_mark_dirty(s_head);
}

void ui_apply_theme(void) {
  window_set_background_color(s_window, theme_bg());
  layer_mark_dirty(s_head);
  layer_mark_dirty(s_bar);
}

void ui_on_interval_changed(void) {
  prv_cache_label_width();
  layer_mark_dirty(s_head);
}

Layer *ui_head_layer(void) { return s_head; }
Layer *ui_bar_layer(void) { return s_bar; }
