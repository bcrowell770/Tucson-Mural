#pragma once
#include <stdbool.h>
#include <stdint.h>
static inline int mural_theme(int hour) { return hour<6||hour>=20?3:hour<10?0:hour<17?1:2; }
static inline int mural_hour(int hour,bool military) { return military?hour:(hour%12?hour%12:12); }
static inline bool weather_valid(int temp,int code,int day,int32_t stamp) {
 return temp>=-9000 && temp<=6000 && code>=0 && code<=99 && (day==0||day==1) && stamp>0;
}
// 0=fresh; 1=stale; 2=unavailable. Reject future data after a clock adjustment.
static inline int weather_age(int32_t now,int32_t stamp) {
 int64_t age=(int64_t)now-stamp;
 return stamp<=0||age< -300||age>=7200?2:age>=3600?1:0;
}
static inline int weather_degrees(int centigrade,bool celsius) {
 int hundredths=celsius?centigrade:(centigrade*9/5+3200);
 return (hundredths+(hundredths<0?-50:50))/100;
}
static inline bool disconnect_alert(bool previous,bool connected,bool enabled) {
 return previous&&!connected&&enabled;
}
