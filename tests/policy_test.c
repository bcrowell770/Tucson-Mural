#include <assert.h>
#include <stdio.h>
#include "../src/c/policy.h"
int main(void) {
 for(int h=0;h<24;h++) {
  assert(mural_hour(h,true)==h);
  assert(mural_hour(h,false)>=1 && mural_hour(h,false)<=12);
 }
 assert(mural_hour(0,false)==12 && mural_hour(12,false)==12 && mural_hour(23,false)==11);
 assert(mural_theme(5)==3 && mural_theme(6)==0 && mural_theme(9)==0 && mural_theme(10)==1);
 assert(mural_theme(16)==1 && mural_theme(17)==2 && mural_theme(19)==2 && mural_theme(20)==3);
 assert(weather_age(10000,10000)==0 && weather_age(13599,10000)==0);
 assert(weather_age(13600,10000)==1 && weather_age(17199,10000)==1);
 assert(weather_age(17200,10000)==2 && weather_age(9000,10000)==2 && weather_age(10000,0)==2);
 assert(weather_degrees(0,false)==32 && weather_degrees(2500,false)==77);
 assert(weather_degrees(-500,true)==-5 && weather_degrees(-150,true)==-2);
 assert(weather_degrees(-9000,false)==-130 && weather_degrees(6000,false)==140);
 assert(weather_valid(2500,0,1,10000));
 assert(!weather_valid(6001,0,1,10000) && !weather_valid(0,100,1,10000));
 assert(!weather_valid(0,0,2,10000) && !weather_valid(0,0,0,0));
 assert(disconnect_alert(true,false,true));
 assert(!disconnect_alert(false,false,true) && !disconnect_alert(false,true,true));
 assert(!disconnect_alert(true,false,false) && !disconnect_alert(true,true,true));
 puts("Clock, lighting, weather age/conversion, disconnect transition policies passed");
}
