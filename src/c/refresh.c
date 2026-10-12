#include <pebble.h>
#include "refresh.h"
#include "config.h"
#include "settings.h"
#include "timer_model.h"
#include "ui.h"

static AppTimer *s_timer;
static bool s_focused = true;

static void prv_fire(void *ctx) {
  s_timer = NULL;
  ECO_LOG("refresh");
  ui_mark_dirty();
  refresh_reschedule();
}

void refresh_cancel(void) {
  if (s_timer) {
    app_timer_cancel(s_timer);
    s_timer = NULL;
  }
}

void refresh_reschedule(void) {
  refresh_cancel();
  if (model_state() != TS_RUNNING || !s_focused) return;
  uint32_t iv = settings_interval_s() * 1000;
  uint32_t phase = (uint32_t)(model_elapsed_ms() % iv);
  s_timer = app_timer_register(iv - phase + REFRESH_SLACK_MS, prv_fire, NULL);
}

void refresh_set_focus(bool in_focus) {
  s_focused = in_focus;
}
