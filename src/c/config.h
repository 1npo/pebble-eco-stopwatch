#pragma once
#include <pebble.h>

#define SCREEN_W PBL_DISPLAY_WIDTH
#define SCREEN_H PBL_DISPLAY_HEIGHT
#define BAR_W ACTION_BAR_WIDTH

// Per-platform layout constants (see docs/plans/002-core-stopwatch.md).
#if PBL_DISPLAY_WIDTH == 144
// aplite, basalt, diorite, flint
#define CONTENT_CX 57
#define LINE_W 102
#define TIMER_BOX_H 44
#define TIMER_PITCH 34
#define TAG_DY 24
#define TAG_X 4
#define SUB_BOX_H 20
#define HEAD_H 112
#define HEAD_Y_CENTER 28
#define HEAD_Y_LAPS 0
#define RULE_Y 114
#define RULE_THICK 1
#define ROW0_Y 3
#define ROW_PITCH 17
#define LAP_ROWS_VISIBLE 3
#define LINE_PITCH 16
#define ICON_SZ 15
#define BAR_ICON_X 129
#define BAR_SELECT_X 129
#define BAR_ICON_Y_UP 28
#define BAR_ICON_Y_SEL 84
#define BAR_ICON_Y_DOWN 140
#define REFRESH_ICON_D 11
#define CAP_TOP 3
#define RESET_THICK 2
#define REFRESH_THICK 1
#define RES_FONT_TIMER RESOURCE_ID_FONT_TIMER_38
#define RES_FONT_TEXT RESOURCE_ID_FONT_TEXT_14
#elif defined(PBL_PLATFORM_CHALK)
#define CONTENT_CX 82
#define LINE_W 102
#define TIMER_BOX_H 44
#define TIMER_PITCH 34
#define TAG_DY 24
#define TAG_X 16
#define SUB_BOX_H 20
#define HEAD_H 112
#define HEAD_Y_CENTER 34
#define HEAD_Y_LAPS 8
#define RULE_Y 122
#define RULE_THICK 1
#define ROW0_Y 3
#define ROW_PITCH 17
#define LAP_ROWS_VISIBLE 3
#define LINE_PITCH 16
#define ICON_SZ 15
#define BAR_ICON_X 158
#define BAR_SELECT_X 160
#define BAR_ICON_Y_UP 50
#define BAR_ICON_Y_SEL 90
#define BAR_ICON_Y_DOWN 130
#define REFRESH_ICON_D 11
#define CAP_TOP 3
#define RESET_THICK 2
#define REFRESH_THICK 1
#define RES_FONT_TIMER RESOURCE_ID_FONT_TIMER_38
#define RES_FONT_TEXT RESOURCE_ID_FONT_TEXT_14
#elif defined(PBL_PLATFORM_EMERY)
#define CONTENT_CX 83
#define LINE_W 148
#define TIMER_BOX_H 64
#define TIMER_PITCH 46
#define TAG_DY 35
#define TAG_X 4
#define SUB_BOX_H 24
#define HEAD_H 156
#define HEAD_Y_CENTER 36
#define HEAD_Y_LAPS 0
#define RULE_Y 158
#define RULE_THICK 2
#define ROW0_Y 4
#define ROW_PITCH 22
#define LAP_ROWS_VISIBLE 3
#define LINE_PITCH 20
#define ICON_SZ 19
#define BAR_ICON_X 183
#define BAR_SELECT_X 183
#define BAR_ICON_Y_UP 38
#define BAR_ICON_Y_SEL 114
#define BAR_ICON_Y_DOWN 190
#define REFRESH_ICON_D 14
#define CAP_TOP 4
#define RESET_THICK 3
#define REFRESH_THICK 2
#define RES_FONT_TIMER RESOURCE_ID_FONT_TIMER_56
#define RES_FONT_TEXT RESOURCE_ID_FONT_TEXT_18
#elif defined(PBL_PLATFORM_GABBRO)
#define CONTENT_CX 130
#define LINE_W 148
#define TIMER_BOX_H 64
#define TIMER_PITCH 46
#define TAG_DY 35
#define TAG_X 16
#define SUB_BOX_H 24
#define HEAD_H 156
#define HEAD_Y_CENTER 52
#define HEAD_Y_LAPS 12
#define RULE_Y 170
#define RULE_THICK 2
#define ROW0_Y 4
#define ROW_PITCH 22
#define LAP_ROWS_VISIBLE 4
#define LINE_PITCH 20
#define ICON_SZ 19
#define BAR_ICON_X 236
#define BAR_SELECT_X 240
#define BAR_ICON_Y_UP 75
#define BAR_ICON_Y_SEL 130
#define BAR_ICON_Y_DOWN 185
#define REFRESH_ICON_D 14
#define CAP_TOP 4
#define RESET_THICK 3
#define REFRESH_THICK 2
#define RES_FONT_TIMER RESOURCE_ID_FONT_TIMER_56
#define RES_FONT_TEXT RESOURCE_ID_FONT_TEXT_18
#else
#error "unsupported platform"
#endif

#define MAX_LAPS 99
#define MAX_ELAPSED_S 3599999 // 999:59:59
#define SHRINK_AT_S 360000 // 100 h
#define REFRESH_SLACK_MS 15

#define ECO_DEBUG_LOG 0
#if ECO_DEBUG_LOG
#define ECO_LOG(...) APP_LOG(APP_LOG_LEVEL_DEBUG, __VA_ARGS__)
#else
#define ECO_LOG(...)
#endif
