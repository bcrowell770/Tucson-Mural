// ES5 PebbleKit JS; canonical Celsius hundredths keep unit changes local to the watch.
module.exports = function(env) {
  var busy=false, lastAttempt=0;
  var cacheKey='tucson.weather.v1';
  function valid(p) {
    return p && typeof p.TempC==='number' && isFinite(p.TempC) && p.TempC>=-9000 && p.TempC<=6000 &&
      typeof p.WeatherCode==='number' && isFinite(p.WeatherCode) && p.WeatherCode>=0 && p.WeatherCode<=99 &&
      (p.IsDay===0||p.IsDay===1) && typeof p.WeatherStamp==='number' && isFinite(p.WeatherStamp) && p.WeatherStamp>0;
  }
  function recent(p) { var age=env.now()-p.WeatherStamp; return age>=-300 && age<7200; }
  function cached() {
    try { var p=JSON.parse(env.storage.getItem(cacheKey)); return valid(p)&&recent(p)?p:null; } catch(e) { return null; }
  }
  function fail() { busy=false; env.log('Weather unavailable; retaining last valid observation'); }
  function refresh(force) {
    var old=cached();
    if(old) env.send(old);
    if(busy || (!force && lastAttempt && env.now()-lastAttempt>=0 && env.now()-lastAttempt<900)) return;
    if(!force && old && env.now()-old.WeatherStamp>=0 && env.now()-old.WeatherStamp<1800) return;
    busy=true; lastAttempt=env.now();
    env.locate(function(pos) {
      if(!pos || !pos.coords || !isFinite(pos.coords.latitude) || !isFinite(pos.coords.longitude) ||
        Math.abs(pos.coords.latitude)>90 || Math.abs(pos.coords.longitude)>180) {fail();return;}
      var url='https://api.open-meteo.com/v1/forecast?latitude='+pos.coords.latitude.toFixed(3)+
        '&longitude='+pos.coords.longitude.toFixed(3)+'&current=temperature_2m,weather_code,is_day&timeformat=unixtime';
      env.get(url,function(data) {
        var c=data && data.current;
        if(!c || typeof c.temperature_2m!=='number') {fail();return;}
        var p={TempC:Math.round(c.temperature_2m*100), WeatherCode:c.weather_code, IsDay:c.is_day, WeatherStamp:c.time};
        if(!valid(p)||!recent(p)) {fail();return;}
        try {env.storage.setItem(cacheKey,JSON.stringify(p));} catch(e) {env.log('Weather cache unavailable');}
        busy=false; env.send(p);
      },fail);
    },fail);
  }
  return {refresh:refresh, valid:valid};
};
