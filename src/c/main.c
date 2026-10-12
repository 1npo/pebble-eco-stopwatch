#include <pebble.h>
#include "config.h"
#include "controller.h"
#include "icons.h"
#include "persist.h"
#include "refresh.h"
#include "settings.h"
#include "theme.h"
#include "ui.h"

static Window *s_window;

static void prv_load(Window *w) {
  ui_create(w);
  ui_apply_theme();
}

static void prv_unload(Window *w) {
  ui_destroy();
}

static void prv_did_focus(bool in_focus) {
  controller_on_focus(in_focus);
}

static void prv_init(void) {
  settings_load();
  persist_load_all();
  theme_set(settings_theme());
  icons_init();
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){ .load = prv_load, .unload = prv_unload });
  controller_init(s_window);
  window_stack_push(s_window, true);
  app_focus_service_subscribe_handlers((AppFocusHandlers){ .did_focus = prv_did_focus });
  refresh_reschedule();
}

static void prv_deinit(void) {
  refresh_cancel();
  persist_save_all();
  app_focus_service_unsubscribe();
  window_destroy(s_window);
  icons_deinit();
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
