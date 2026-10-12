#include <pebble.h>
#include "settings.h"
#include "controller.h"
#include "persist.h"

#define DEFAULT_INTERVAL_S 10
#define DEFAULT_THEME      PBL_IF_COLOR_ELSE(THEME_COLOR, THEME_WOB)

static uint8_t s_interval_s = DEFAULT_INTERVAL_S;
static Theme s_theme = DEFAULT_THEME;

static uint8_t prv_valid_interval(int32_t v) {
  static const uint8_t ok[] = { 1, 2, 5, 10, 15, 20, 30, 60 };
  for (uint32_t i = 0; i < sizeof(ok); i++) {
    if (ok[i] == v) return ok[i];
  }
  return DEFAULT_INTERVAL_S;
}

static Theme prv_valid_theme(int32_t v) {
  return (v < 0 || v > THEME_COLOR) ? DEFAULT_THEME : (Theme)v;
}

void settings_load(void) {
  s_interval_s = DEFAULT_INTERVAL_S;
  s_theme = DEFAULT_THEME;
  if (persist_exists(PKEY_INTERVAL)) {
    s_interval_s = prv_valid_interval(persist_read_int(PKEY_INTERVAL));
  }
  if (persist_exists(PKEY_THEME)) {
    s_theme = prv_valid_theme(persist_read_int(PKEY_THEME));
  }
}

uint8_t settings_interval_s(void) {
  return s_interval_s;
}

Theme settings_theme(void) {
  return s_theme;
}

// Clay select values arrive as C strings; accept int tuples too. -1 if unparsable.
static int32_t prv_tuple_int(const Tuple *t) {
  switch (t->type) {
    case TUPLE_CSTRING: {
      const char *p = t->value->cstring;
      if (*p < '0' || *p > '9') return -1;
      int32_t v = 0;
      while (*p >= '0' && *p <= '9' && v < 100000) {
        v = v * 10 + (*p - '0');
        p++;
      }
      return v;
    }
    case TUPLE_INT:
      switch (t->length) {
        case 1: return t->value->int8;
        case 2: return t->value->int16;
        default: return t->value->int32;
      }
    case TUPLE_UINT:
      switch (t->length) {
        case 1: return t->value->uint8;
        case 2: return t->value->uint16;
        default: return (int32_t)t->value->uint32;
      }
    default:
      return -1;
  }
}

static void prv_inbox_received(DictionaryIterator *iter, void *ctx) {
  bool changed = false;
  Tuple *t = dict_find(iter, MESSAGE_KEY_RefreshInterval);
  if (t) {
    s_interval_s = prv_valid_interval(prv_tuple_int(t));
    persist_write_int(PKEY_INTERVAL, s_interval_s);
    changed = true;
  }
  t = dict_find(iter, MESSAGE_KEY_Theme);
  if (t) {
    s_theme = prv_valid_theme(prv_tuple_int(t));
    persist_write_int(PKEY_THEME, s_theme);
    changed = true;
  }
  if (changed) controller_on_settings_changed();
}

void settings_messaging_init(void) {
  app_message_register_inbox_received(prv_inbox_received);
  app_message_open(128, 64);
}
