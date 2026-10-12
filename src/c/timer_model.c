#include <pebble.h>
#include "timer_model.h"
#include "config.h"

static struct {
  TimerState state;
  int64_t start_ms;
  int64_t accum_ms;
} s_m;

static int64_t s_last_now_ms;

void model_init(void) {
  s_m.state = TS_STOPPED;
  s_m.start_ms = 0;
  s_m.accum_ms = 0;
}

TimerState model_state(void) {
  return s_m.state;
}

int64_t model_now_ms(void) {
  time_t s;
  uint16_t ms;
  time_ms(&s, &ms);
  int64_t now = (int64_t)s * 1000 + ms;
  if (now < s_last_now_ms) {
    now = s_last_now_ms;
  }
  s_last_now_ms = now;
  return now;
}

static int64_t prv_segment_ms(void) {
  int64_t seg = model_now_ms() - s_m.start_ms;
  return seg > 0 ? seg : 0;
}

int64_t model_elapsed_ms(void) {
  return s_m.accum_ms + (s_m.state == TS_RUNNING ? prv_segment_ms() : 0);
}

uint32_t model_elapsed_s(void) {
  int64_t s = model_elapsed_ms() / 1000;
  return s > MAX_ELAPSED_S ? MAX_ELAPSED_S : (uint32_t)s;
}

void model_start(void) {
  if (s_m.state == TS_RUNNING) return;
  s_m.start_ms = model_now_ms();
  s_m.state = TS_RUNNING;
}

void model_pause(void) {
  if (s_m.state != TS_RUNNING) return;
  s_m.accum_ms += prv_segment_ms();
  s_m.state = TS_PAUSED;
}

void model_reset(void) {
  model_init();
}

void model_get_snapshot(TimerState *st, int64_t *start_ms, int64_t *accum_ms) {
  *st = s_m.state;
  *start_ms = s_m.start_ms;
  *accum_ms = s_m.accum_ms;
}

void model_restore(TimerState st, int64_t start_ms, int64_t accum_ms) {
  s_m.state = st;
  s_m.start_ms = start_ms;
  s_m.accum_ms = accum_ms;
}
