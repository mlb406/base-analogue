#include <pebble.h>

static Window *window;
static Layer *circle_layer, *bg_layer, *hands_layer;
static TextLayer *date_layer;

static const GPathInfo MINUTE_HAND_POINTS = {
  7,
  (GPoint []) {
    {-6, 5},
    {6, 5},
    {4, -60},
    {3, -64},
    {0, -67},
    {-3, -64},
    {-4, -60},
  }
};

static const GPathInfo HOUR_HAND_POINTS = {
  5, (GPoint []){
    {-6, 5},
    {6, 5},
    {4, -43},
    {0, -46},
    {-4, -43},
  }
};

static GPath *s_minute_arrow, *s_hour_arrow;

static void date_update() {
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
  
  static char date_buffer[] = "00";
  
  strftime(date_buffer, sizeof(date_buffer), "%d", t);
  
  text_layer_set_text(date_layer, date_buffer);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  date_update();
  layer_mark_dirty(window_get_root_layer(window));
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
  
  int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
  
  int second_hand_length = 70;
  
  GPoint second_hand = {
    .x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.y,
  };
  
  graphics_draw_line(ctx, center, second_hand);
  
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  
  gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
  gpath_draw_filled(ctx, s_minute_arrow);
  gpath_draw_outline(ctx, s_minute_arrow);
  
  gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
  gpath_draw_filled(ctx, s_hour_arrow);
  gpath_draw_outline(ctx, s_hour_arrow);
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  
  graphics_fill_circle(ctx, center, 2);
}

static void bg_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  int i;
  for (i = 0; i < 12; i++) {    
    int32_t minute_angle = TRIG_MAX_ANGLE * i / 12;
    GPoint minute_hand_1 = {
      .x = (int16_t)(sin_lookup(minute_angle) * (int32_t)70 / TRIG_MAX_RATIO) + center.x,
      .y = (int16_t)(-cos_lookup(minute_angle) * (int32_t)70 / TRIG_MAX_RATIO) + center.y,
    };
    
    GPoint minute_hand_2 = {
      .x = (int16_t)(sin_lookup(minute_angle) * (int32_t)65 / TRIG_MAX_RATIO) + center.x,
      .y = (int16_t)(-cos_lookup(minute_angle) * (int32_t)65 / TRIG_MAX_RATIO) + center.y,
    };
    graphics_context_set_stroke_color(ctx, GColorBlack);
    
    
    graphics_draw_line(ctx, minute_hand_1, minute_hand_2);
    
  }
  int i2;
  for (i2 = 0; i2 < 60; i2++) {
    int32_t bg_angle = TRIG_MAX_ANGLE * i2 / 60;
    GPoint minute_hand_4 = {
      .x = (int16_t)(sin_lookup(bg_angle) * (int32_t)70 / TRIG_MAX_RATIO) + center.x,
      .y = (int16_t)(-cos_lookup(bg_angle) * (int32_t)70 / TRIG_MAX_RATIO) + center.y,
    };
    
    GPoint minute_hand_3 = {
      .x = (int16_t)(sin_lookup(bg_angle) * (int32_t)68 / TRIG_MAX_RATIO) + center.x,
      .y = (int16_t)(-cos_lookup(bg_angle) * (int32_t)68 / TRIG_MAX_RATIO) + center.y,
    };
    graphics_context_set_stroke_color(ctx, GColorBlack);
    
    
    graphics_draw_line(ctx, minute_hand_3, minute_hand_4);
  }
  
}

static void circle_create_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  
  graphics_fill_circle(ctx, center, (bounds.size.w / 2) - 4);
}

static void main_window_load() {
  circle_layer = layer_create(GRect(0, 0, 144, 168));
  layer_set_update_proc(circle_layer, circle_create_proc);
  layer_add_child(window_get_root_layer(window), circle_layer);
  
  bg_layer = layer_create(GRect(0, 0, 144, 168));
  layer_set_update_proc(bg_layer, bg_proc);
  layer_add_child(window_get_root_layer(window), bg_layer);
  
  date_layer = text_layer_create(GRect(6, 72, 134, 20));
  text_layer_set_text_color(date_layer, GColorBlack);
  text_layer_set_background_color(date_layer, GColorWhite);
  text_layer_set_text_alignment(date_layer, GTextAlignmentRight);
  text_layer_set_text(date_layer, "00");
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));
  
  hands_layer = layer_create(GRect(0, 0, 144, 168));
  layer_set_update_proc(hands_layer, hands_update_proc);
  layer_add_child(window_get_root_layer(window), hands_layer);
}

static void main_window_unload() {
  layer_destroy(circle_layer);
  layer_destroy(bg_layer);
  layer_destroy(hands_layer);
}

static void init() {
  window = window_create();
  
  window_set_background_color(window, GColorBlack);
  
  window_set_window_handlers(window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  
  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
  
  s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
  s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);
  
  GPoint center = GPoint(72, 84);
  gpath_move_to(s_minute_arrow, center);
  gpath_move_to(s_hour_arrow, center);
  
  window_stack_push(window, true);
  
  date_update();
}

static void deinit() {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
  return 0;
}