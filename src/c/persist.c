#include <pebble.h>
#include "persist.h"
#include "timer_model.h"

void persist_load_all(void) {
  if (persist_read_int(PKEY_SCHEMA) != PERSIST_SCHEMA_VERSION) return;
  PersistTimer t;
  if (persist_read_data(PKEY_TIMER, &t, sizeof(t)) != (int)sizeof(t)) return;
  if (t.state > TS_PAUSED || t.accum_ms < 0) return;
  model_restore((TimerState)t.state, t.start_ms, t.accum_ms);
}

void persist_save_all(void) {
  PersistTimer t;
  memset(&t, 0, sizeof(t));
  TimerState st;
  model_get_snapshot(&st, &t.start_ms, &t.accum_ms);
  t.state = (uint8_t)st;
  persist_write_int(PKEY_SCHEMA, PERSIST_SCHEMA_VERSION);
  persist_write_data(PKEY_TIMER, &t, sizeof(t));
}
