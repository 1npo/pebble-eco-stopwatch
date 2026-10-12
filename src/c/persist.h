#pragma once
#include <pebble.h>

#define PERSIST_SCHEMA_VERSION 1

#define PKEY_SCHEMA 1 // int, value PERSIST_SCHEMA_VERSION
#define PKEY_TIMER 2 // data, PersistTimer
#define PKEY_LAPS_A 3 // data, uint32_t[<=50]  laps 1..50   (003)
#define PKEY_LAPS_B 4 // data, uint32_t[<=49]  laps 51..99  (003)
#define PKEY_INTERVAL 5 // int, seconds          (004)
#define PKEY_THEME 6 // int, Theme            (004)
#define PKEY_SAFEGUARD 7 // int, minutes, 0 = off

typedef struct {
	uint8_t state; // TimerState
	uint8_t lap_count; // 0..99 (003; 0 in 002)
	uint8_t reserved[6];
	int64_t start_ms;
	int64_t accum_ms;
} PersistTimer; // 24 bytes

void persist_load_all(void);
void persist_save_all(void);
