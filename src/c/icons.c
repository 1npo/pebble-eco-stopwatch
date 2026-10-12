#include <pebble.h>
#include "icons.h"
#include "config.h"
#include "theme.h"

// Paths are built for the one size used on this platform (ICON_SZ, and the
// two cycle-icon variants), relative to (0,0), then moved to the centre.
#define S ICON_SZ
static GPoint s_play_pts[3]  = {{-S / 3, -S / 2}, {-S / 3, S / 2}, {S / 2, 0}};
static GPoint s_up_pts[3]    = {{0, -S / 3}, {-S / 2, S / 3}, {S / 2, S / 3}};
static GPoint s_down_pts[3]  = {{0, S / 3}, {-S / 2, -S / 3}, {S / 2, -S / 3}};
// Reset ring (d = ICON_SZ, thick = RESET_THICK): arrowhead points left.
static GPoint s_rst_pts[3]   = {{-3, -S / 2 + RESET_THICK / 2},
                                {1, -S / 2 - 2},
                                {1, -S / 2 + RESET_THICK + 2}};
// Refresh ring (d = REFRESH_ICON_D, thick = REFRESH_THICK): mirrored, points right.
static GPoint s_rfr_pts[3]   = {{3, -REFRESH_ICON_D / 2 + REFRESH_THICK / 2},
                                {-1, -REFRESH_ICON_D / 2 - 2},
                                {-1, -REFRESH_ICON_D / 2 + REFRESH_THICK + 2}};

static GPathInfo s_play_info, s_up_info, s_down_info, s_rst_info, s_rfr_info;
static GPath *s_play, *s_up, *s_down, *s_rst, *s_rfr;

static GPath *prv_make(GPathInfo *info, GPoint *pts) {
  info->num_points = 3;
  info->points = pts;
  return gpath_create(info);
}

void icons_init(void) {
  s_play = prv_make(&s_play_info, s_play_pts);
  s_up = prv_make(&s_up_info, s_up_pts);
  s_down = prv_make(&s_down_info, s_down_pts);
  s_rst = prv_make(&s_rst_info, s_rst_pts);
  s_rfr = prv_make(&s_rfr_info, s_rfr_pts);
}

void icons_deinit(void) {
  gpath_destroy(s_play);
  gpath_destroy(s_up);
  gpath_destroy(s_down);
  gpath_destroy(s_rst);
  gpath_destroy(s_rfr);
}

static void prv_draw(GContext *ctx, GPath *p, GPoint c) {
  gpath_move_to(p, c);
  gpath_draw_filled(ctx, p);
}

void icon_play(GContext *ctx, GPoint c, int16_t s)  { prv_draw(ctx, s_play, c); }
void icon_up(GContext *ctx, GPoint c, int16_t s)    { prv_draw(ctx, s_up, c); }
void icon_down(GContext *ctx, GPoint c, int16_t s)  { prv_draw(ctx, s_down, c); }

void icon_pause(GContext *ctx, GPoint c, int16_t s) {
  int16_t w = s / 4, gap = s / 4;
  int16_t x = c.x - (2 * w + gap) / 2;
  graphics_fill_rect(ctx, GRect(x, c.y - s / 2, w, s), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(x + w + gap, c.y - s / 2, w, s), 0, GCornerNone);
}

void icon_flag(GContext *ctx, GPoint c, int16_t s, GColor bg) {
  graphics_fill_rect(ctx, GRect(c.x - s / 2, c.y - s / 2, 2, s), 0, GCornerNone);
  int16_t k = (s - 3) / 4;
  int16_t x0 = c.x - s / 2 + 2, y0 = c.y - s / 2;
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 4; col++) {
      if ((row + col) % 2 == 0) {
        graphics_fill_rect(ctx, GRect(x0 + col * k, y0 + row * k, k, k), 0, GCornerNone);
      }
    }
  }
  // The context has no fill-colour getter; the flag is only drawn on the bar.
  graphics_context_set_stroke_color(ctx, theme_bar_fg());
  graphics_draw_rect(ctx, GRect(x0, y0, 4 * k, 3 * k));
}

void icon_cycle(GContext *ctx, GPoint c, int16_t d, int16_t thick, bool clockwise) {
  GRect r = GRect(c.x - d / 2, c.y - d / 2, d, d);
  if (clockwise) {
    graphics_fill_radial(ctx, r, GOvalScaleModeFitCircle, thick,
                         DEG_TO_TRIGANGLE(60), DEG_TO_TRIGANGLE(360));
    prv_draw(ctx, s_rfr, c);
  } else {
    graphics_fill_radial(ctx, r, GOvalScaleModeFitCircle, thick,
                         DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(300));
    prv_draw(ctx, s_rst, c);
  }
}

// Solid shield: full width for the top half, tapering to a point.
void icon_shield(GContext *ctx, GPoint c, int16_t d) {
  int16_t half = d * 9 / 20;
  int16_t flat = d * 11 / 20;
  for (int16_t y = 0; y < d; y++) {
    int16_t hw = y < flat ? half : half * (d - y) / (d - flat);
    if (hw < 1) hw = 1;
    graphics_fill_rect(ctx, GRect(c.x - hw, c.y - d / 2 + y, 2 * hw, 1), 0, GCornerNone);
  }
}
