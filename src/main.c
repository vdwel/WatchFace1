#include <pebble.h>

  
Window *my_window;
TextLayer *s_time_layer;
TextLayer *s_date_layer;


static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";
  static char buffer2[]= "XX-XX-XXXX\nWeek XX";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof(buffer), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof(buffer), "%I:%M", tick_time);
  }

  // Write the date in buffer2
  strftime(buffer2, sizeof(buffer2), "%d-%m-%Y\nWeek %U", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  text_layer_set_text(s_date_layer, buffer2);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

void ConfigureTextLayer(GColor TextColor, GColor BackgroundColor, GColor dateTextColor, GColor dateBackgroundColor) {
   // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 55, 144, 50));
  s_date_layer = text_layer_create(GRect(0, 105, 144, 40));
  text_layer_set_background_color(s_time_layer, BackgroundColor);
  text_layer_set_text_color(s_time_layer, TextColor);
  text_layer_set_background_color(s_date_layer, dateBackgroundColor);
  text_layer_set_text_color(s_date_layer, dateTextColor);  
}

static void main_window_load(Window *window) {
  #ifdef PBL_COLOR
    ConfigureTextLayer(GColorRed, GColorCeleste, GColorCobaltBlue, GColorWhite);
    window_set_background_color(window, GColorRajah);
  #else 
    ConfigureTextLayer(GColorClear, GColorBlack, GColorBlack, GColorClear);
    window_set_background_color(window, GColorWhite);
  #endif
  
  text_layer_set_text(s_time_layer, "00:00");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);


  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));

}

static void main_window_unload(Window *window) {
    // Destroy TextLayer
    text_layer_destroy(s_time_layer);
}

void handle_init(void) {
  my_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(my_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(my_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

void handle_deinit(void) {
//  text_layer_destroy(text_layer);
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
