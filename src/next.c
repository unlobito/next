#include <stdlib.h>
#include <stdio.h>
#include <pebble.h>
#include "config.c"

static Window *window;
static TextLayer *currenttime;
static TextLayer *currentdate;

static TextLayer *currentcourse;
static TextLayer *remainingcourse;
static TextLayer *remainingtext;

unsigned int classpercent;
unsigned int drawnclasspercent;
course courseinprogress;

static int determine_daytime(struct tm *tick_time) {
	unsigned int daytime = (tick_time->tm_sec + (tick_time->tm_min*60) + (tick_time->tm_hour*60*60));
	
	return daytime;
}

static course detectcourse(unsigned int daytime) {
	if (daytime < school.start_time || daytime > school.end_time) {
		course classisover = {
			.code = 1,
			.name = "class is over"
		};
		
		return classisover;
	}
	
	unsigned int i = 0;
	for (i=0;i<sizeof(courses)/sizeof(courses[0]);i++) {
		if (daytime >= courses[i].start_time && daytime <= courses[i].end_time) {
			return courses[i];
		}
	}
	
	i = 0;
	for (i=0;i<sizeof(courses)/sizeof(courses[0]);i++) {
		if (daytime >= courses[i].end_time && daytime <= courses[i+1].start_time) {
			static char nextname[] = "next\nsillylongclassnamenothingshouldbethislong";
			snprintf(nextname, sizeof(nextname), "next:\n%s", courses[i+1].name);
			course transition = {
				.code = 3,
				.name = nextname,
				.start_time = courses[i].end_time,
				.end_time = courses[i+1].start_time
			};
			
			return transition;
		}
	}
	
	course exception = {
		.code = 2,
		.name = "exception\noccurred"
	};
	
	return exception;
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
	/* time + date formatting code lifted from pebble-sdk-examples/simplicity
	   https://github.com/pebble/pebble-sdk-examples/blob/master/watchfaces/simplicity/src/simplicity.c */
	static char time_text[] = "00:00";
	strftime(time_text, sizeof(time_text), "%R", tick_time);
	text_layer_set_text(currenttime, time_text);
	
	static char date_text[] = "Xxxxxxxxx 00";
	strftime(date_text, sizeof(date_text), "%B %e", tick_time);
	text_layer_set_text(currentdate, date_text);
	
	courseinprogress = detectcourse(determine_daytime(tick_time));
	text_layer_set_text(currentcourse, courseinprogress.name);
	
	if (courseinprogress.code == 0 || courseinprogress.code == 3) {
		classpercent = ((determine_daytime(tick_time) - courseinprogress.start_time) * 100 / (courseinprogress.end_time - courseinprogress.start_time));
		unsigned int timeremainingmin = ((courseinprogress.end_time - determine_daytime(tick_time))/60);
		unsigned int timeremainingrem = ((courseinprogress.end_time - determine_daytime(tick_time)) % 60);
		
		if (timeremainingmin <= 4) {
			static char timeremaining[] = "000 min 000 sec";
			snprintf(timeremaining, sizeof(timeremaining), "%i min %i sec", timeremainingmin, timeremainingrem);
			text_layer_set_text(remainingcourse, timeremaining);
		} else {
			static char timeremaining[] = "000 min";
			snprintf(timeremaining, sizeof(timeremaining), "%i min", timeremainingmin);
			text_layer_set_text(remainingcourse, timeremaining);
		}
		
		text_layer_set_text(remainingtext, "remaining");
	} else {
		text_layer_set_text(remainingcourse, "");
		text_layer_set_text(remainingtext, "");
	}
	
	if (drawnclasspercent != classpercent) { // try and save battery life by reducing redraws
		Layer *window_layer = window_get_root_layer(window);
		layer_mark_dirty(window_layer);
		drawnclasspercent = classpercent;
	}
}

void draw_layer(Layer *layer, GContext *gctxt) {
	graphics_context_set_fill_color(gctxt, GColorBlack);
	graphics_fill_rect(gctxt, GRect(0, 0, 144, 168), 0, GCornerNone);
	
	if (classpercent != 0) {
		graphics_context_set_fill_color(gctxt, GColorBlack);
		graphics_fill_rect(gctxt, GRect(0, 60, 144, 5), 0, GCornerNone);
		
		graphics_context_set_fill_color(gctxt, GColorWhite);
		graphics_fill_rect(gctxt, GRect(0, 60, (classpercent*144/100), 5), 0, GCornerNone);
	}
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	layer_set_update_proc(window_layer, draw_layer);
	GRect bounds = layer_get_bounds(window_layer);

	currenttime = text_layer_create(GRect(0, 0, 144, 168-92));
	text_layer_set_text_color(currenttime, GColorWhite);
	text_layer_set_background_color(currenttime, GColorClear);
	text_layer_set_font(currenttime, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
	text_layer_set_text_alignment(currenttime, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(currenttime));
	
	currentdate = text_layer_create(GRect(0, 35, 144, 168-92));
	text_layer_set_text_color(currentdate, GColorWhite);
	text_layer_set_background_color(currentdate, GColorClear);
	text_layer_set_font(currentdate, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(currentdate, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(currentdate));
	
	currentcourse = text_layer_create(GRect(7, 60, 144-7, 60));
	text_layer_set_text_color(currentcourse, GColorWhite);
	text_layer_set_background_color(currentcourse, GColorClear);
	text_layer_set_font(currentcourse, fonts_get_system_font(FONT_KEY_GOTHIC_28));
	text_layer_set_text_alignment(currentcourse, GTextAlignmentLeft);
	layer_add_child(window_layer, text_layer_get_layer(currentcourse));
	
	remainingcourse = text_layer_create(GRect(0, 130, 144, 60));
	text_layer_set_text_color(remainingcourse, GColorWhite);
	text_layer_set_background_color(remainingcourse, GColorClear);
	text_layer_set_font(remainingcourse, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(remainingcourse, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(remainingcourse));
	
	remainingtext = text_layer_create(GRect(0, 145, 144, 60));
	text_layer_set_text_color(remainingtext, GColorWhite);
	text_layer_set_background_color(remainingtext, GColorClear);
	text_layer_set_font(remainingtext, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(remainingtext, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(remainingtext));
	text_layer_set_text(remainingtext, "remaining");
}

static void window_unload(Window *window) {
	text_layer_destroy(currenttime);
}

static void init(void) {
	window = window_create();
	window_set_fullscreen(window, true);
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
	const bool animated = true;
	window_stack_push(window, animated);
}

static void deinit(void) {
	window_destroy(window);
}

int main(void) {
	init();

	app_event_loop();
	deinit();
}
