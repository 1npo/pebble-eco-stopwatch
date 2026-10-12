#include <pebble.h>
#include "anim.h"
#include "config.h"
#include "ui.h"

#define SHOW_MS 300
#define HIDE_MS 250

static Animation *s_anim;
static bool s_target_shown;

static void prv_stopped(Animation *a, bool finished, void *ctx) {
  s_anim = NULL;
  if (finished && !s_target_shown) {
    layer_set_hidden(ui_laps_layer(), true);
  }
}

static void prv_run(bool show, uint32_t ms) {
  if (s_anim) animation_unschedule(s_anim);  // fires prv_stopped(finished=false)
  s_target_shown = show;
  Layer *head = ui_head_layer();
  Layer *laps = ui_laps_layer();
  if (show) layer_set_hidden(laps, false);
  GRect head_to = layer_get_frame(head);
  head_to.origin.y = ui_head_y(show);
  GRect laps_to = layer_get_frame(laps);
  laps_to.origin.y = show ? ui_rule_y() : SCREEN_H;
  Animation *a = property_animation_get_animation(
      property_animation_create_layer_frame(head, NULL, &head_to));
  Animation *b = property_animation_get_animation(
      property_animation_create_layer_frame(laps, NULL, &laps_to));
  // Duration and curve go on the children; the spawn ignores them.
  animation_set_duration(a, ms);
  animation_set_curve(a, AnimationCurveEaseInOut);
  animation_set_duration(b, ms);
  animation_set_curve(b, AnimationCurveEaseInOut);
  s_anim = animation_spawn_create(a, b, NULL);
  animation_set_handlers(s_anim, (AnimationHandlers){ .stopped = prv_stopped }, NULL);
  animation_schedule(s_anim);
}

void anim_show_laps(void) { prv_run(true, SHOW_MS); }

void anim_hide_laps(void) { prv_run(false, HIDE_MS); }

void anim_cancel(void) {
  if (s_anim) animation_unschedule(s_anim);
}
