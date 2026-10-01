#include <pebble.h>
#include <stdbool.h>
#include <stdio.h>

#define NUM_PHASES 8
#define OUTLINE_THICKNESS 20

static Window *s_main_window;
static Layer *s_ring_layer;
static GPath *s_ring_path;
static GPoint s_ring_pts[10];
static GPathInfo s_ring_info = {10, s_ring_pts};
static TextLayer *s_time_layer;
static TextLayer *s_status_layer;
static TextLayer *s_pomodoro_count_layer;
static int countdowns[8] = {1500, 300, 1500, 300, 1500, 300, 1500, 900};
static int curr_countdown = 1500;
static int pomodoro_count = 1;
static bool paused = true;

static const char *phase_name(int phase) {
  if ((phase - 1) % NUM_PHASES == NUM_PHASES - 1)
    return "LONG BREAK";
  return ((phase - 1) % 2 == 0) ? "FOCUS" : "SHORT BREAK";
}

static void draw_time() {
  static char s_time_buffer[6];
  snprintf(s_time_buffer, sizeof(s_time_buffer), "%02d:%02d",
           curr_countdown / 60, curr_countdown % 60);
  text_layer_set_text(s_time_layer, s_time_buffer);
}

static void draw_status() {
  text_layer_set_text(s_status_layer,
                      paused ? "PAUSED" : phase_name(pomodoro_count));
}

static void draw_pomodoro_count() {
  static char s_pc_buffer[11];
  if (pomodoro_count > 9999) {
    snprintf(s_pc_buffer, sizeof(s_pc_buffer), "bruh chill");
  } else {
    snprintf(s_pc_buffer, sizeof(s_pc_buffer), "%d", pomodoro_count);
  }
  text_layer_set_text(s_pomodoro_count_layer, s_pc_buffer);
}

static void draw_ring(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  s_ring_path = gpath_create(&s_ring_info);
  s_ring_pts[0] = GPoint(-1, 0);
  s_ring_pts[1] = GPoint(b.size.w, 0);
  s_ring_pts[2] = GPoint(b.size.w, b.size.h);
  s_ring_pts[3] = GPoint(OUTLINE_THICKNESS - 1, b.size.h);
  s_ring_pts[4] = GPoint(OUTLINE_THICKNESS - 1, b.size.h - OUTLINE_THICKNESS);
  s_ring_pts[5] =
      GPoint(b.size.w - OUTLINE_THICKNESS, b.size.h - OUTLINE_THICKNESS);
  s_ring_pts[6] = GPoint(b.size.w - OUTLINE_THICKNESS, OUTLINE_THICKNESS);
  s_ring_pts[7] = GPoint(OUTLINE_THICKNESS, OUTLINE_THICKNESS);
  s_ring_pts[8] = GPoint(OUTLINE_THICKNESS, b.size.h);
  s_ring_pts[9] = GPoint(-1, b.size.h);
  graphics_context_set_fill_color(ctx, GColorIslamicGreen);
  gpath_draw_filled(ctx, s_ring_path);
}

static void update_time() {
  if (paused)
    return;
  curr_countdown--;
  if (curr_countdown < 0) {
    curr_countdown = countdowns[pomodoro_count % 8];
    pomodoro_count++;
    draw_status();
  }
  draw_time();
}

static void tick_handler(struct tm *_, TimeUnits units_changed) {
  update_time();
}

static void click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (click_recognizer_get_button_id(recognizer)) {
  case BUTTON_ID_UP:
    curr_countdown = countdowns[pomodoro_count % 8];
    pomodoro_count++;
    draw_time();
    draw_status();
    draw_pomodoro_count();
    break;
  case BUTTON_ID_SELECT:
    paused = !paused;
    paused ? tick_timer_service_unsubscribe()
           : tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    draw_status();
    break;
  case BUTTON_ID_DOWN:
    curr_countdown = countdowns[pomodoro_count % 8];
    pomodoro_count++;
    draw_time();
    draw_status();
    draw_pomodoro_count();
    break;
  default:
    break;
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, click_handler);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect bounds = layer_get_bounds(window_layer);
  s_ring_layer = layer_create(bounds);
  s_time_layer =
      text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
  s_status_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(116, 104), bounds.size.w, 50));
  s_pomodoro_count_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(174, 156), bounds.size.w, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_background_color(s_status_layer, GColorClear);
  text_layer_set_background_color(s_pomodoro_count_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text_color(s_status_layer, GColorWhite);
  text_layer_set_text_color(s_pomodoro_count_layer, GColorWhite);
  text_layer_set_font(s_time_layer,
                      fonts_get_system_font(FONT_KEY_LECO_36_BOLD_NUMBERS));
  text_layer_set_font(s_status_layer,
                      fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_font(s_pomodoro_count_layer,
                      fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_status_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_pomodoro_count_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, s_ring_layer);
  layer_set_update_proc(s_ring_layer, draw_ring);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_status_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_pomodoro_count_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorRed);
  window_set_window_handlers(s_main_window, (WindowHandlers){
                                                .load = main_window_load,
                                                .unload = main_window_unload,
                                            });
  window_stack_push(s_main_window, true);
  window_set_click_config_provider(s_main_window, click_config_provider);
  draw_time();
  draw_status();
  draw_pomodoro_count();
}

static void deinit() {
  window_destroy(s_main_window);
  if (!paused)
    tick_timer_service_unsubscribe();
}

int main(int argc, char *argv[]) {
  init();
  app_event_loop();
  deinit();
}
