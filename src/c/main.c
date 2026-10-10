#include <pebble.h>
#include "layout_profile.h"
#include "lighting.h"
#include "policy.h"
#ifndef POC_THEME
#define POC_THEME -1
#endif

#ifndef POC_HOUR
#define POC_HOUR -1
#endif
#ifndef POC_MINUTE
#define POC_MINUTE 42
#endif
static int s_theme=-1;
static int s_digits[4]={-1,-1,-1,-1};
static char s_date[20],s_short_date[12];
typedef struct { int32_t temp,code,day,stamp; } Weather;
static Weather s_weather;
static bool s_celsius=false,s_show_bt=true,s_vibrate=false,s_connected=true,s_low_vision=false;
static time_t s_request_time;
static AppTimer *s_request_timer;
static const char *temperature(void) {
 static char text[16];
 if (POC_HOUR>=0) return "78\xC2\xB0";
 if (weather_age(time(NULL),s_weather.stamp)==2) return "--\xC2\xB0";
 snprintf(text,sizeof(text),"%d\xC2\xB0",weather_degrees(s_weather.temp,s_celsius));
 return text;
}

static Window *s_window;
static Layer *s_canvas;
static GBitmap *s_background;
static GBitmap *s_tiles[4];
static GColor foreground(void) { return s_theme==3 ? GColorPastelYellow : GColorBlack; }

static GBitmap *load_image(uint32_t id,int width,int height) {
 bool tile=width==PROFILE_TILE_WIDTH;
 int row_bytes=tile?width:(width+1)/2;
 int offset=tile?0:16;
 GBitmap *bitmap=gbitmap_create_blank(GSize(width,height),tile?GBitmapFormat8Bit:GBitmapFormat4BitPalette);
 if (!bitmap) { APP_LOG(APP_LOG_LEVEL_ERROR,"Image allocation failed %lu",(unsigned long)id); return NULL; }
 uint8_t *data=gbitmap_get_data(bitmap);
 uint16_t stride=gbitmap_get_bytes_per_row(bitmap);
 ResHandle handle=resource_get_handle(id);
 if (!tile && resource_load_byte_range(handle,0,(uint8_t *)gbitmap_get_palette(bitmap),16)!=16) {gbitmap_destroy(bitmap); return NULL;}
 for (int row=0;row<height;row++) {
  if (resource_load_byte_range(handle,offset+row*row_bytes,data+row*stride,row_bytes)!=(size_t)row_bytes) {
   APP_LOG(APP_LOG_LEVEL_ERROR,"Image read failed %lu",(unsigned long)id);
   gbitmap_destroy(bitmap); return NULL;
  }
  if (tile) {
   for (int x=0;x<width;x++) {
    uint8_t value=data[row*stride+x];
    if (value==0x40) data[row*stride+x]=foreground().argb;
    else if (value>=0x80) data[row*stride+x]=LIGHTING[s_theme][(value&0xc0)==0x80][value&0x3f];
   }
  }
 }
 return bitmap;
}
static void request_weather(void *unused) {
 if (!s_connected || POC_HOUR>=0) return;
 time_t now=time(NULL);
 int64_t elapsed=(int64_t)now-s_request_time;
 int age=weather_age(now,s_weather.stamp);
 // Retry unavailable/stale data every 15 minutes; normal cadence is 30 minutes.
 if (s_request_time && elapsed>=0 && elapsed<(age?900:1800)) return;
 DictionaryIterator *iter;
 if (app_message_outbox_begin(&iter)!=APP_MSG_OK) return;
 dict_write_uint8(iter,MESSAGE_KEY_RequestWeather,1);
 if (app_message_outbox_send()==APP_MSG_OK) s_request_time=now;
}
static void scheduled_request(void *unused) {s_request_timer=NULL;request_weather(NULL);}
static void connection_changed(bool connected) {
 if (disconnect_alert(s_connected,connected,s_vibrate)) {
  vibes_short_pulse(); APP_LOG(APP_LOG_LEVEL_INFO,"Disconnect pulse");
 }
 bool reconnected=!s_connected&&connected;
 s_connected=connected;
 APP_LOG(APP_LOG_LEVEL_INFO,"Phone connection %d",connected);
 if (s_canvas) layer_mark_dirty(s_canvas);
 if (reconnected && !s_request_timer) s_request_timer=app_timer_register(1500,scheduled_request,NULL);
}
static bool read_integer(DictionaryIterator *iter,uint32_t key,int32_t *value) {
 Tuple *t=dict_find(iter,key);
 if (!t || (t->type!=TUPLE_INT && t->type!=TUPLE_UINT)) return false;
 if (t->length==1) *value=t->type==TUPLE_INT?t->value->int8:t->value->uint8;
 else if (t->length==2) *value=t->type==TUPLE_INT?t->value->int16:t->value->uint16;
 else if (t->length==4) {
  if (t->type==TUPLE_UINT && t->value->uint32>INT32_MAX) return false;
  *value=t->value->int32;
 } else return false;
 return true;
}
static void inbox(DictionaryIterator *iter,void *context) {
 int32_t value;
 bool changed=false;
 if (read_integer(iter,MESSAGE_KEY_TemperatureUnit,&value) && (value==0||value==1)) {s_celsius=value==1;persist_write_bool(101,s_celsius);changed=true;}
 if (read_integer(iter,MESSAGE_KEY_ShowBtIcon,&value) && (value==0||value==1)) {s_show_bt=value!=0;persist_write_bool(102,s_show_bt);changed=true;}
 if (read_integer(iter,MESSAGE_KEY_BtVibrate,&value) && (value==0||value==1)) {s_vibrate=value!=0;persist_write_bool(103,s_vibrate);changed=true;}
 if (read_integer(iter,MESSAGE_KEY_LowVisionMode,&value) && (value==0||value==1)) {s_low_vision=value!=0;persist_write_bool(105,s_low_vision);changed=true;}
 Weather next;
 if (read_integer(iter,MESSAGE_KEY_TempC,&next.temp) && read_integer(iter,MESSAGE_KEY_WeatherCode,&next.code) &&
     read_integer(iter,MESSAGE_KEY_IsDay,&next.day) && read_integer(iter,MESSAGE_KEY_WeatherStamp,&next.stamp)) {
  if (weather_valid(next.temp,next.code,next.day,next.stamp) && weather_age(time(NULL),next.stamp)!=2) {
   s_weather=next;persist_write_data(104,&s_weather,sizeof(s_weather));
   APP_LOG(APP_LOG_LEVEL_INFO,"Weather %ld C/100 code %ld",(long)next.temp,(long)next.code);
  }
 }
 if (changed) APP_LOG(APP_LOG_LEVEL_INFO,"Settings C:%d icon:%d vibe:%d low:%d",s_celsius,s_show_bt,s_vibrate,s_low_vision);
 if (s_canvas) layer_mark_dirty(s_canvas);
}
static void draw_weather(GContext *ctx,int x,int y) {
 int code=POC_HOUR>=0?0:s_weather.code;
 int age=POC_HOUR>=0?0:weather_age(time(NULL),s_weather.stamp);
 graphics_context_set_fill_color(ctx,foreground());
 graphics_context_set_stroke_color(ctx,foreground());
 if (age==2) {graphics_draw_line(ctx,GPoint(x-3,y),GPoint(x+3,y));return;}
 graphics_context_set_stroke_width(ctx,s_low_vision?2:1);
 if (code<=1) {
  graphics_fill_circle(ctx,GPoint(x,y),s_low_vision?4:3);
  if (!s_weather.day && POC_HOUR<0) {
   graphics_context_set_fill_color(ctx,s_theme==3?GColorOxfordBlue:GColorLightGray);
   graphics_fill_circle(ctx,GPoint(x+2,y-1),2);
  } else {
   graphics_draw_line(ctx,GPoint(x,y-5),GPoint(x,y-7));
   graphics_draw_line(ctx,GPoint(x,y+5),GPoint(x,y+7));
   graphics_draw_line(ctx,GPoint(x-5,y),GPoint(x-7,y));
   graphics_draw_line(ctx,GPoint(x+5,y),GPoint(x+7,y));
  }
 } else {
  graphics_fill_circle(ctx,GPoint(x-3,y),3);graphics_fill_circle(ctx,GPoint(x,y-2),4);
  graphics_fill_circle(ctx,GPoint(x+4,y),3);graphics_fill_rect(ctx,GRect(x-4,y,9,3),0,GCornerNone);
  if (code>=95) {graphics_draw_line(ctx,GPoint(x+1,y+3),GPoint(x-1,y+5));graphics_draw_line(ctx,GPoint(x-1,y+5),GPoint(x+1,y+5));graphics_draw_line(ctx,GPoint(x+1,y+5),GPoint(x-1,y+7));}
  else if ((code>=71&&code<=77)||code==85||code==86) {
   graphics_draw_pixel(ctx,GPoint(x-3,y+5));graphics_draw_pixel(ctx,GPoint(x+3,y+5));
  } else if (code>=51) {
   graphics_draw_line(ctx,GPoint(x-2,y+4),GPoint(x-3,y+6));graphics_draw_line(ctx,GPoint(x+3,y+4),GPoint(x+2,y+6));
  } else if (code>=45) graphics_draw_line(ctx,GPoint(x-5,y+5),GPoint(x+5,y+5));
 }
 graphics_context_set_stroke_width(ctx,1);
 if (age==1) {graphics_context_set_fill_color(ctx,foreground());graphics_fill_circle(ctx,GPoint(x,y+(s_low_vision?11:9)),1);}
}
static void draw_disconnect(GContext *ctx) {
 if (s_connected||!s_show_bt) return;
 int x=PROFILE_WIDTH/2,y=PROFILE_TILE_TOP/2;
 graphics_context_set_stroke_color(ctx,foreground());
 graphics_draw_line(ctx,GPoint(x,y-6),GPoint(x,y+6));
 graphics_draw_line(ctx,GPoint(x,y-6),GPoint(x+4,y-3));
 graphics_draw_line(ctx,GPoint(x+4,y-3),GPoint(x-4,y+3));
 graphics_draw_line(ctx,GPoint(x-4,y-3),GPoint(x+4,y+3));
 graphics_draw_line(ctx,GPoint(x+4,y+3),GPoint(x,y+6));
 graphics_draw_line(ctx,GPoint(x-6,y+6),GPoint(x+6,y-6));
}
static void draw(Layer *layer,GContext *ctx) {
 graphics_context_set_fill_color(ctx,GColorWhite);
 graphics_fill_rect(ctx,layer_get_bounds(layer),0,GCornerNone);
 graphics_context_set_compositing_mode(ctx,GCompOpAssign);
 if (s_background) graphics_draw_bitmap_in_rect(ctx,s_background,GRect(0,0,PROFILE_WIDTH,PROFILE_HEIGHT));
 graphics_context_set_compositing_mode(ctx,GCompOpSet);
 for (int i=0;i<4;i++) {
  if (s_tiles[i]) graphics_draw_bitmap_in_rect(ctx,s_tiles[i],GRect(TILE_X[i],PROFILE_TILE_TOP,PROFILE_TILE_WIDTH,PROFILE_TILE_HEIGHT));
 }
 for (int i=0;i<2;i++) {
  GPoint p=GPoint(PROFILE_COLON_X,PROFILE_COLON_Y+i*PROFILE_COLON_STEP);
  if (s_theme==3) {
   graphics_context_set_fill_color(ctx,foreground());
   if (i==0) {
    graphics_fill_circle(ctx,p,PROFILE_COLON_RADIUS);
    graphics_context_set_fill_color(ctx,GColorOxfordBlue); graphics_fill_circle(ctx,GPoint(p.x+2,p.y-1),PROFILE_COLON_RADIUS-1);
   } else {
    graphics_fill_rect(ctx,GRect(p.x-1,p.y-3,3,7),0,GCornerNone);
    graphics_fill_rect(ctx,GRect(p.x-3,p.y-1,7,3),0,GCornerNone);
   }
  } else {
   graphics_context_set_fill_color(ctx,GColorBlack); graphics_fill_circle(ctx,p,PROFILE_COLON_RADIUS);
   graphics_context_set_fill_color(ctx,GColorOrange); graphics_fill_circle(ctx,p,PROFILE_COLON_RADIUS-1);
   graphics_context_set_fill_color(ctx,GColorYellow); graphics_fill_circle(ctx,GPoint(p.x-1,p.y-1),1);
  }
 }
 // Larger footer stays beneath the unchanged mural numerals.
 GFont footer_font=fonts_get_system_font(s_low_vision?PROFILE_LOW_FONT:PROFILE_FONT);
 int inset=s_low_vision?PROFILE_LOW_INSET:PROFILE_DATE_X;
 int top=s_low_vision?PROFILE_LOW_TOP:PROFILE_DATE_TOP;
 int weather_y=s_low_vision?top+(PROFILE_WIDTH==144?13:17):PROFILE_WEATHER_Y;
 GSize temp_size=graphics_text_layout_get_content_size(temperature(),footer_font,GRect(0,0,80,32),GTextOverflowModeTrailingEllipsis,GTextAlignmentLeft);
 int weather_x=PROFILE_WIDTH-inset-temp_size.w-16;
 const char *date=s_date;
 int date_width=PROFILE_DATE_WIDTH;
 if (s_low_vision) {
  date_width=weather_x-9-6-inset;
  GSize date_size=graphics_text_layout_get_content_size(date,footer_font,GRect(0,0,200,32),GTextOverflowModeTrailingEllipsis,GTextAlignmentLeft);
  if (date_size.w>date_width) date=s_short_date;

 }
 graphics_context_set_text_color(ctx,foreground());
 graphics_draw_text(ctx,date,footer_font,GRect(inset,top,date_width,32),GTextOverflowModeTrailingEllipsis,GTextAlignmentLeft,NULL);
 draw_weather(ctx,weather_x,weather_y);
 draw_disconnect(ctx);
 graphics_draw_text(ctx,temperature(),footer_font,GRect(PROFILE_WIDTH-inset-64,top,64,32),GTextOverflowModeTrailingEllipsis,GTextAlignmentRight,NULL);
 if(s_low_vision) APP_LOG(APP_LOG_LEVEL_INFO,"Large footer %s width %d temp %d",date,date_width,temp_size.w);

}
static void update_time(struct tm *now, TimeUnits units) {
 int hour=POC_HOUR>=0?POC_HOUR:now->tm_hour;
 int minute=POC_HOUR>=0?POC_MINUTE:now->tm_min;
 int theme=POC_THEME>=0?POC_THEME:mural_theme(hour);
 bool changed=theme!=s_theme;
 s_theme=theme;
 if (changed || !s_background) {
  if (s_background) gbitmap_destroy(s_background);
  s_background=load_image(BACKGROUND_IDS[s_theme],PROFILE_WIDTH,PROFILE_HEIGHT);
 }
 int display_hour=mural_hour(hour,clock_is_24h_style());
 // Review builds preserve the exact requested hour irrespective of emulator preference.
 if (POC_HOUR>=0) display_hour=hour;
 int digits[4]={display_hour/10,display_hour%10,minute/10,minute%10};
 for (int i=0;i<4;i++) if (changed || digits[i]!=s_digits[i] || !s_tiles[i]) {
  if (s_tiles[i]) gbitmap_destroy(s_tiles[i]);
  s_tiles[i]=load_image(TILE_IDS[digits[i]],PROFILE_TILE_WIDTH,PROFILE_TILE_HEIGHT);
  s_digits[i]=digits[i];
 }
 if (POC_HOUR>=0) {snprintf(s_date,sizeof(s_date),"THU OCT 1");snprintf(s_short_date,sizeof(s_short_date),"OCT 1");}
 else {
  strftime(s_date,sizeof(s_date),"%a %b %e",now);
  char month[5];strftime(month,sizeof(month),"%b",now);
  snprintf(s_short_date,sizeof(s_short_date),"%s %d",month,now->tm_mday);
  for (char *p=s_short_date;*p;p++) if (*p>='a' && *p<='z') *p-=32;
  for (char *p=s_date;*p;p++) if (*p>='a' && *p<='z') *p-=32;
 }
 request_weather(NULL);
 if (s_canvas) layer_mark_dirty(s_canvas);
 APP_LOG(APP_LOG_LEVEL_INFO,"Tucson theme %d %02d:%02d display %02d; %s; free heap %lu",s_theme,hour,minute,display_hour,s_date,(unsigned long)heap_bytes_free());
}
static void load(Window *window) {
 s_canvas=layer_create(layer_get_bounds(window_get_root_layer(window)));
 if (s_canvas) {layer_set_update_proc(s_canvas,draw);layer_add_child(window_get_root_layer(window),s_canvas);}
 time_t stamp=time(NULL); struct tm *now=localtime(&stamp);
 update_time(now,MINUTE_UNIT);
 if (POC_HOUR<0) tick_timer_service_subscribe(MINUTE_UNIT,update_time);
}
static void unload(Window *window) {
 tick_timer_service_unsubscribe();
 connection_service_unsubscribe();
 if(s_request_timer) {app_timer_cancel(s_request_timer);s_request_timer=NULL;}
 app_message_deregister_callbacks();
 if (s_canvas) layer_destroy(s_canvas);
 if (s_background) gbitmap_destroy(s_background);
 for (int i=0;i<4;i++) if (s_tiles[i]) gbitmap_destroy(s_tiles[i]);
}
int main(void) {
 if (persist_exists(101)) s_celsius=persist_read_bool(101);
 if (persist_exists(102)) s_show_bt=persist_read_bool(102);
 if (persist_exists(103)) s_vibrate=persist_read_bool(103);
 if (persist_read_data(104,&s_weather,sizeof(s_weather))!=sizeof(s_weather) ||
     !weather_valid(s_weather.temp,s_weather.code,s_weather.day,s_weather.stamp)) s_weather=(Weather){0};
 if (persist_exists(105)) s_low_vision=persist_read_bool(105);
 s_connected=connection_service_peek_pebble_app_connection();
 connection_service_subscribe((ConnectionHandlers){.pebble_app_connection_handler=connection_changed});
 app_message_register_inbox_received(inbox);
 app_message_open(256,64);
 s_window=window_create(); if (!s_window) return 1;
 window_set_window_handlers(s_window,(WindowHandlers){.load=load,.unload=unload});
 window_stack_push(s_window,true); app_event_loop(); window_destroy(s_window);
}

