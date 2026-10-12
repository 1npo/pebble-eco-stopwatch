#pragma once
#include <pebble.h>

#define SCREEN_W PBL_DISPLAY_WIDTH
#define SCREEN_H PBL_DISPLAY_HEIGHT
#define BAR_W ACTION_BAR_WIDTH

// Per-platform layout constants (see docs/plans/002-core-stopwatch.md).
#if PBL_DISPLAY_WIDTH == 144
  // aplite, basalt, diorite, flint
  #define CONTENT_CX       57
  #define LINE_W           102
  #define TIMER_BOX_H      34
  #define TIMER_SMALL_DY   3
  #define SUB_Y            29
  #define SUB_BOX_H        20
  #define HEAD_H           49
  #define HEAD_Y_CENTER    59
  #define HEAD_Y_LAPS      0
  #define RULE_Y           48
  #define RULE_THICK       1
  #define ROW0_Y           3
  #define ROW_PITCH        17
  #define LAP_ROWS_VISIBLE 6
  #define SAFE_DY          16
  #define SAFE_ROWS_LOST   1
  #define ICON_SZ          15
  #define BAR_ICON_X       129
  #define BAR_SELECT_X     129
  #define BAR_ICON_Y_UP    28
  #define BAR_ICON_Y_SEL   84
  #define BAR_ICON_Y_DOWN  140
  #define REFRESH_ICON_D   11
  #define CAP_TOP          3
  #define RESET_THICK      2
  #define REFRESH_THICK    1
  #define RES_FONT_TIMER       RESOURCE_ID_FONT_TIMER_26
  #define RES_FONT_TIMER_SMALL RESOURCE_ID_FONT_TIMER_22
  #define RES_FONT_TEXT        RESOURCE_ID_FONT_TEXT_14
#elif defined(PBL_PLATFORM_CHALK)
  #define CONTENT_CX       82
  #define LINE_W           102
  #define TIMER_BOX_H      34
  #define TIMER_SMALL_DY   3
  #define SUB_Y            29
  #define SUB_BOX_H        20
  #define HEAD_H           49
  #define HEAD_Y_CENTER    65
  #define HEAD_Y_LAPS      21
  #define RULE_Y           69
  #define RULE_THICK       1
  #define ROW0_Y           3
  #define ROW_PITCH        17
  #define LAP_ROWS_VISIBLE 4
  #define SAFE_DY          16
  #define SAFE_ROWS_LOST   0
  #define ICON_SZ          15
  #define BAR_ICON_X       158
  #define BAR_SELECT_X     160
  #define BAR_ICON_Y_UP    50
  #define BAR_ICON_Y_SEL   90
  #define BAR_ICON_Y_DOWN  130
  #define REFRESH_ICON_D   11
  #define CAP_TOP          3
  #define RESET_THICK      2
  #define REFRESH_THICK    1
  #define RES_FONT_TIMER       RESOURCE_ID_FONT_TIMER_26
  #define RES_FONT_TIMER_SMALL RESOURCE_ID_FONT_TIMER_22
  #define RES_FONT_TEXT        RESOURCE_ID_FONT_TEXT_14
#elif defined(PBL_PLATFORM_EMERY)
  #define CONTENT_CX       83
  #define LINE_W           148
  #define TIMER_BOX_H      46
  #define TIMER_SMALL_DY   2
  #define SUB_Y            42
  #define SUB_BOX_H        24
  #define HEAD_H           66
  #define HEAD_Y_CENTER    79
  #define HEAD_Y_LAPS      0
  #define RULE_Y           66
  #define RULE_THICK       2
  #define ROW0_Y           4
  #define ROW_PITCH        22
  #define LAP_ROWS_VISIBLE 7
  #define SAFE_DY          20
  #define SAFE_ROWS_LOST   1
  #define ICON_SZ          19
  #define BAR_ICON_X       183
  #define BAR_SELECT_X     183
  #define BAR_ICON_Y_UP    38
  #define BAR_ICON_Y_SEL   114
  #define BAR_ICON_Y_DOWN  190
  #define REFRESH_ICON_D   14
  #define CAP_TOP          4
  #define RESET_THICK      3
  #define REFRESH_THICK    2
  #define RES_FONT_TIMER       RESOURCE_ID_FONT_TIMER_38
  #define RES_FONT_TIMER_SMALL RESOURCE_ID_FONT_TIMER_34
  #define RES_FONT_TEXT        RESOURCE_ID_FONT_TEXT_18
#elif defined(PBL_PLATFORM_GABBRO)
  #define CONTENT_CX       130
  #define LINE_W           148
  #define TIMER_BOX_H      46
  #define TIMER_SMALL_DY   2
  #define SUB_Y            42
  #define SUB_BOX_H        24
  #define HEAD_H           66
  #define HEAD_Y_CENTER    95
  #define HEAD_Y_LAPS      19
  #define RULE_Y           85
  #define RULE_THICK       2
  #define ROW0_Y           4
  #define ROW_PITCH        22
  #define LAP_ROWS_VISIBLE 6
  #define SAFE_DY          20
  #define SAFE_ROWS_LOST   0
  #define ICON_SZ          19
  #define BAR_ICON_X       236
  #define BAR_SELECT_X     240
  #define BAR_ICON_Y_UP    75
  #define BAR_ICON_Y_SEL   130
  #define BAR_ICON_Y_DOWN  185
  #define REFRESH_ICON_D   14
  #define CAP_TOP          4
  #define RESET_THICK      3
  #define REFRESH_THICK    2
  #define RES_FONT_TIMER       RESOURCE_ID_FONT_TIMER_38
  #define RES_FONT_TIMER_SMALL RESOURCE_ID_FONT_TIMER_34
  #define RES_FONT_TEXT        RESOURCE_ID_FONT_TEXT_18
#else
  #error "unsupported platform"
#endif

#define MAX_LAPS         99
#define MAX_ELAPSED_S    3599999  // 999:59:59
#define SHRINK_AT_S      360000   // 100 h
#define REFRESH_SLACK_MS 15

#define ECO_DEBUG_LOG 0
#if ECO_DEBUG_LOG
  #define ECO_LOG(...) APP_LOG(APP_LOG_LEVEL_DEBUG, __VA_ARGS__)
#else
  #define ECO_LOG(...)
#endif
