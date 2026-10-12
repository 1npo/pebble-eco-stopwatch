#include <pebble.h>
#include "controller.h"
#include "anim.h"
#include "config.h"
#include "refresh.h"
#include "settings.h"
#include "theme.h"
#include "timer_model.h"
#include "ui.h"

#define SCROLL_REPEAT_MS 150

static Window *s_window;
static uint8_t s_scroll_first;
static bool s_scroll_cfg;  // click config currently has repeating UP/DOWN

static uint8_t prv_newest_first(void) {
  uint8_t n = model_lap_count();
  return n > LAP_ROWS_VISIBLE ? n - LAP_ROWS_VISIBLE : 0;
}

UiMode controller_mode(void) {
  switch (model_state()) {
    case TS_RUNNING: return UI_RUNNING;
    case TS_PAUSED:
      return model_lap_count() > LAP_ROWS_VISIBLE ? UI_PAUSED_SCROLL : UI_PAUSED;
    default:         return UI_STOPPED;
  }
}

uint8_t controller_scroll_first(void) {
  return controller_mode() == UI_PAUSED_SCROLL ? s_scroll_first : prv_newest_first();
}

static void prv_click_config(void *ctx);

static void prv_sync_click_config(void) {
  bool scroll = controller_mode() == UI_PAUSED_SCROLL;
  if (scroll != s_scroll_cfg) {
    s_scroll_cfg = scroll;
    window_set_click_config_provider(s_window, prv_click_config);
  }
}

static void prv_after_transition(void) {
  prv_sync_click_config();
  refresh_reschedule();
  layer_mark_dirty(ui_bar_layer());
  ui_mark_dirty();
}

static void prv_up(ClickRecognizerRef ref, void *ctx) {
  switch (controller_mode()) {
    case UI_RUNNING:
      if (model_add_lap()) {
        if (model_lap_count() == 1) anim_show_laps();
        layer_mark_dirty(ui_bar_layer());
        ui_mark_laps_dirty();
      }
      break;
    case UI_PAUSED_SCROLL:
      if (s_scroll_first > 0) {
        s_scroll_first--;
        ui_mark_laps_dirty();
      }
      break;
    default:
      break;
  }
}

static void prv_select(ClickRecognizerRef ref, void *ctx) {
  switch (controller_mode()) {
    case UI_STOPPED:
    case UI_PAUSED:
    case UI_PAUSED_SCROLL:
      model_start();
      break;
    case UI_RUNNING:
      model_pause();
      s_scroll_first = prv_newest_first();
      break;
  }
  prv_after_transition();
}

static void prv_down(ClickRecognizerRef ref, void *ctx) {
  switch (controller_mode()) {
    case UI_RUNNING:
    case UI_PAUSED: {
      bool had = model_lap_count() > 0;
      model_reset();
      if (had) anim_hide_laps();
      prv_after_transition();
      break;
    }
    case UI_PAUSED_SCROLL:
      if (s_scroll_first < prv_newest_first()) {
        s_scroll_first++;
        ui_mark_laps_dirty();
      }
      break;
    default:
      break;
  }
}

static void prv_click_config(void *ctx) {
  if (s_scroll_cfg) {
    window_single_repeating_click_subscribe(BUTTON_ID_UP, SCROLL_REPEAT_MS, prv_up);
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, SCROLL_REPEAT_MS, prv_down);
  } else {
    window_single_click_subscribe(BUTTON_ID_UP, prv_up);
    window_single_click_subscribe(BUTTON_ID_DOWN, prv_down);
  }
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select);
}

void controller_init(Window *w) {
  s_window = w;
  s_scroll_first = prv_newest_first();
  s_scroll_cfg = controller_mode() == UI_PAUSED_SCROLL;
  window_set_click_config_provider(w, prv_click_config);
}

void controller_on_focus(bool in_focus) {
  refresh_set_focus(in_focus);
  if (in_focus) {
    ui_mark_dirty();
    refresh_reschedule();
  } else {
    refresh_cancel();
  }
}

void controller_on_settings_changed(void) {
  theme_set(settings_theme());
  ui_apply_theme();
  ui_on_interval_changed();
  refresh_reschedule();
}
