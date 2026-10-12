#include <pebble.h>
#include "controller.h"
#include "refresh.h"
#include "timer_model.h"
#include "ui.h"

UiMode controller_mode(void) {
  switch (model_state()) {
    case TS_RUNNING: return UI_RUNNING;
    case TS_PAUSED:  return UI_PAUSED;
    default:         return UI_STOPPED;
  }
}

static void prv_after_transition(void) {
  refresh_reschedule();
  layer_mark_dirty(ui_bar_layer());
  ui_mark_dirty();
}

static void prv_up(ClickRecognizerRef ref, void *ctx) {
  // UI_RUNNING: lap (003). Ignored in 002.
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
      break;
  }
  prv_after_transition();
}

static void prv_down(ClickRecognizerRef ref, void *ctx) {
  UiMode m = controller_mode();
  if (m != UI_RUNNING && m != UI_PAUSED) return;
  model_reset();
  prv_after_transition();
}

static void prv_click_config(void *ctx) {
  window_single_click_subscribe(BUTTON_ID_UP, prv_up);
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down);
}

void controller_init(Window *w) {
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
  ui_on_interval_changed();
  refresh_reschedule();
}
