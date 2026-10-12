#pragma once
#include <pebble.h>

typedef enum {
	TS_STOPPED = 0,
	TS_RUNNING = 1,
	TS_PAUSED = 2,
	TS_TERMINATED = 3
} TimerState;

void model_init(void);
TimerState model_state(void);
int64_t model_now_ms(void);
int64_t model_elapsed_ms(void);
uint32_t model_elapsed_s(void);
void model_start(void);
void model_pause(void);
void model_terminate(int64_t at_ms);
void model_reset(void);
void model_get_snapshot(TimerState *st, int64_t *start_ms, int64_t *accum_ms);
void model_restore(TimerState st, int64_t start_ms, int64_t accum_ms);
uint8_t model_lap_count(void);
uint32_t model_lap_ms(uint8_t index);
bool model_add_lap(void);
void model_restore_laps(const uint32_t *laps, uint8_t count);
