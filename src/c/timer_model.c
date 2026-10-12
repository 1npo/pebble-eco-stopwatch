#include <pebble.h>
#include "timer_model.h"
#include "config.h"

static struct {
	TimerState state;
	int64_t start_ms;
	int64_t accum_ms;
} s_m;

static int64_t s_last_now_ms;
static uint32_t s_laps[MAX_LAPS];
static uint8_t s_lap_count;

void model_init(void)
{
	s_m.state = TS_STOPPED;
	s_m.start_ms = 0;
	s_m.accum_ms = 0;
	s_lap_count = 0;
}

TimerState model_state(void)
{
	return s_m.state;
}

int64_t model_now_ms(void)
{
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

static int64_t prv_segment_ms(void)
{
	int64_t seg = model_now_ms() - s_m.start_ms;
	return seg > 0 ? seg : 0;
}

int64_t model_elapsed_ms(void)
{
	return s_m.accum_ms + (s_m.state == TS_RUNNING ? prv_segment_ms() : 0);
}

uint32_t model_elapsed_s(void)
{
	int64_t s = model_elapsed_ms() / 1000;
	return s > MAX_ELAPSED_S ? MAX_ELAPSED_S : (uint32_t)s;
}

void model_start(void)
{
	if (s_m.state == TS_RUNNING || s_m.state == TS_TERMINATED)
		return;
	s_m.start_ms = model_now_ms();
	s_m.state = TS_RUNNING;
}

void model_pause(void)
{
	if (s_m.state != TS_RUNNING)
		return;
	s_m.accum_ms += prv_segment_ms();
	s_m.state = TS_PAUSED;
}

// Freezes the timer at at_ms of elapsed time (the safeguard limit).
void model_terminate(int64_t at_ms)
{
	if (s_m.state != TS_RUNNING)
		return;
	s_m.accum_ms = at_ms;
	s_m.state = TS_TERMINATED;
}

void model_reset(void)
{
	model_init();
}

void model_get_snapshot(TimerState *st, int64_t *start_ms, int64_t *accum_ms)
{
	*st = s_m.state;
	*start_ms = s_m.start_ms;
	*accum_ms = s_m.accum_ms;
}

void model_restore(TimerState st, int64_t start_ms, int64_t accum_ms)
{
	s_m.state = st;
	s_m.start_ms = start_ms;
	s_m.accum_ms = accum_ms;
}

uint8_t model_lap_count(void)
{
	return s_lap_count;
}

uint32_t model_lap_ms(uint8_t index)
{
	return index < s_lap_count ? s_laps[index] : 0;
}

bool model_add_lap(void)
{
	if (s_m.state != TS_RUNNING || s_lap_count >= MAX_LAPS)
		return false;
	s_laps[s_lap_count++] = (uint32_t)model_elapsed_ms();
	return true;
}

void model_restore_laps(const uint32_t *laps, uint8_t count)
{
	if (count > MAX_LAPS)
		count = MAX_LAPS;
	memcpy(s_laps, laps, count * sizeof(uint32_t));
	s_lap_count = count;
}
