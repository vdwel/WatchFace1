#include <pebble.h>
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_SUNRISE 2
#define KEY_SUNSET 3

Window *my_window;
TextLayer *s_time_layer;
TextLayer *s_date_layer;
TextLayer *s_url_layer;

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

  // Get weather update every 30 minutes
  if(tick_time->tm_min % 1 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

void ConfigureTextLayer(GColor TextColor, GColor BackgroundColor, GColor dateTextColor, GColor dateBackgroundColor) {
   // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 35, 144, 50));
  s_date_layer = text_layer_create(GRect(0, 85, 144, 40));
  s_url_layer = text_layer_create(GRect(0, 125, 144, 40));

  text_layer_set_background_color(s_time_layer, BackgroundColor);
  text_layer_set_text_color(s_time_layer, TextColor);
  text_layer_set_background_color(s_date_layer, dateBackgroundColor);
  text_layer_set_text_color(s_date_layer, dateTextColor);  
  text_layer_set_background_color(s_url_layer, dateBackgroundColor);
  text_layer_set_text_color(s_url_layer, dateTextColor);  

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
  text_layer_set_text(s_url_layer, "loading...");


  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_font(s_url_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_url_layer, GTextAlignmentCenter);



  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_url_layer));


  update_time();
}

static void main_window_unload(Window *window) {
    // Destroy TextLayer
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_url_layer);

}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char sunrise_buffer[6];
  static char sunset_buffer[6];
  static char weather_layer_buffer[32];
  int rawtime;

  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)t->value->int32);
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    case KEY_SUNRISE:
      rawtime = t->value->int32;
      time_t sunrisetime = (time_t)rawtime;
      struct tm *sunrise = localtime(&sunrisetime);
      // Write the current hours and minutes into the buffer
      if(clock_is_24h_style() == true) {
        // Use 24 hour format
        strftime(sunrise_buffer, sizeof(sunrise_buffer), "%H:%M", sunrise);
      } else {
        // Use 12 hour format
        strftime(sunrise_buffer, sizeof(sunrise_buffer), "%I:%M", sunrise);
      }
      break;
    case KEY_SUNSET:
      rawtime = t->value->int32;
      time_t sunsettime = (time_t)rawtime;
      struct tm *sunset = localtime(&sunsettime);
      // Write the current hours and minutes into the buffer
      if(clock_is_24h_style() == true) {
        // Use 24 hour format
        strftime(sunset_buffer, sizeof(sunset_buffer), "%H:%M", sunset);
      } else {
        // Use 12 hour format
        strftime(sunset_buffer, sizeof(sunset_buffer), "%I:%M", sunset);
      }
      break;

    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  // Assemble full string and display
//  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", sunrise_buffer, sunset_buffer);
  text_layer_set_text(s_url_layer, weather_layer_buffer);

}


static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
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
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
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
