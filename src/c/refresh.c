#include <pebble.h>
#include "refresh.h"
#include "config.h"
#include "controller.h"
#include "settings.h"
#include "timer_model.h"
#include "ui.h"

static AppTimer *s_timer;
static bool s_focused = true;

static void prv_fire(void *ctx) {
  s_timer = NULL;
  if (controller_safeguard_check()) return;
  // Some clocks (basalt emulator) fire the timer up to ~1 s before the boundary.
  // Skip the redraw then; the re-arm below fires again on the boundary.
  uint32_t iv = settings_interval_s() * 1000;
  if (model_elapsed_ms() % iv <= iv / 2) {
    ECO_LOG("refresh");
    ui_mark_dirty();
  }
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
  uint32_t delay = iv - phase + REFRESH_SLACK_MS;
  if (settings_safeguard_min()) {
    // Also wake at the safeguard limit, even between refreshes.
    int64_t left = (int64_t)settings_safeguard_min() * 60000 - model_elapsed_ms();
    if (left < 1) left = 1;
    if (left + REFRESH_SLACK_MS < delay) delay = (uint32_t)left + REFRESH_SLACK_MS;
  }
  s_timer = app_timer_register(delay, prv_fire, NULL);
}

void refresh_set_focus(bool in_focus) {
  s_focused = in_focus;
}
