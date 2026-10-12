#include <pebble.h>
#include "persist.h"
#include "config.h"
#include "timer_model.h"

#define LAPS_PER_CHUNK 50

void persist_load_all(void) {
  if (persist_read_int(PKEY_SCHEMA) != PERSIST_SCHEMA_VERSION) return;
  PersistTimer t;
  if (persist_read_data(PKEY_TIMER, &t, sizeof(t)) != (int)sizeof(t)) return;
  if (t.state > TS_PAUSED || t.accum_ms < 0) return;
  model_restore((TimerState)t.state, t.start_ms, t.accum_ms);

  uint32_t laps[MAX_LAPS];
  uint8_t count = t.lap_count > MAX_LAPS ? MAX_LAPS : t.lap_count;
  uint8_t got = 0;
  if (count > 0) {
    uint8_t a = count > LAPS_PER_CHUNK ? LAPS_PER_CHUNK : count;
    int n = persist_read_data(PKEY_LAPS_A, laps, a * sizeof(uint32_t));
    got = n > 0 ? (uint8_t)(n / sizeof(uint32_t)) : 0;
    if (got == a && count > a) {
      int m = persist_read_data(PKEY_LAPS_B, laps + a, (count - a) * sizeof(uint32_t));
      got += m > 0 ? (uint8_t)(m / sizeof(uint32_t)) : 0;
    }
  }
  model_restore_laps(laps, got);
}

void persist_save_all(void) {
  PersistTimer t;
  memset(&t, 0, sizeof(t));
  TimerState st;
  model_get_snapshot(&st, &t.start_ms, &t.accum_ms);
  t.state = (uint8_t)st;
  uint8_t count = model_lap_count();
  t.lap_count = count;
  persist_write_int(PKEY_SCHEMA, PERSIST_SCHEMA_VERSION);
  persist_write_data(PKEY_TIMER, &t, sizeof(t));

  uint32_t laps[MAX_LAPS];
  for (uint8_t i = 0; i < count; i++) laps[i] = model_lap_ms(i);
  if (count > 0) {
    uint8_t a = count > LAPS_PER_CHUNK ? LAPS_PER_CHUNK : count;
    persist_write_data(PKEY_LAPS_A, laps, a * sizeof(uint32_t));
  } else {
    persist_delete(PKEY_LAPS_A);
  }
  if (count > LAPS_PER_CHUNK) {
    persist_write_data(PKEY_LAPS_B, laps + LAPS_PER_CHUNK,
                       (count - LAPS_PER_CHUNK) * sizeof(uint32_t));
  } else {
    persist_delete(PKEY_LAPS_B);
  }
}
