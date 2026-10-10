var assert=require('assert'),vm=require('vm'),fs=require('fs');
var makeWeather=require('../src/pkjs/weather');
function fixture() {
 var f={clock:100000,sent:[],requests:0,locations:0,data:{},failLocation:false,failHttp:false};
 f.engine=makeWeather({now:function(){return f.clock;},storage:{getItem:function(k){return f.data[k];},setItem:function(k,v){f.data[k]=v;}},
  send:function(p){f.sent.push(p);},log:function(){},locate:function(ok,fail){f.locations++;if(f.failLocation)fail();else ok({coords:{latitude:32.222,longitude:-110.975}});},
  get:function(url,ok,fail){f.requests++;assert(url.indexOf('latitude=32.222')>0);if(f.failHttp)fail();else ok({current:{temperature_2m:25.5,weather_code:0,is_day:1,time:f.clock}});}});
 return f;
}
var f=fixture();f.engine.refresh(false);assert.equal(f.sent[0].TempC,2550);assert.equal(f.requests,1);
f.clock+=60;f.engine.refresh(false);assert.equal(f.requests,1);assert.equal(f.sent.length,2);
f.clock+=1800;f.engine.refresh(false);assert.equal(f.requests,2);
f=fixture();f.failLocation=true;f.engine.refresh(false);assert.equal(f.sent.length,0);assert.equal(f.requests,0);
f.clock+=60;f.engine.refresh(false);assert.equal(f.locations,1);f.clock+=900;f.engine.refresh(false);assert.equal(f.locations,2);
f=fixture();f.engine.refresh(false);f.clock+=4000;f.failHttp=true;f.engine.refresh(false);assert.equal(f.sent.length,2);
f.clock+=8000;f.engine.refresh(false);assert.equal(f.sent.length,2); // expired cache never sent
assert(!f.engine.valid({TempC:NaN,WeatherCode:0,IsDay:1,WeatherStamp:1}));
assert(!f.engine.valid({TempC:20,WeatherCode:'0',IsDay:1,WeatherStamp:1}));
// Exercise actual entry point event routing, serialized delivery, retry and Clay values.
var events={},sent=[],timers=[],store={},pending=[],geo=0;
function Clay(){this.generateUrl=function(){return 'data:text/html,config';};this.getSettings=function(s){return JSON.parse(s);};}
var context={require:function(name){if(name==='@rebble/clay')return Clay;if(name==='./config')return require('../src/pkjs/config');return makeWeather;},
 localStorage:{getItem:function(k){return store[k];},setItem:function(k,v){store[k]=v;}},console:console,Date:Date,isFinite:isFinite,
 setTimeout:function(f){timers.push(f);},navigator:{geolocation:{getCurrentPosition:function(ok,fail){geo++;fail();}}},
 Pebble:{addEventListener:function(n,f){events[n]=f;},sendAppMessage:function(p,ok,fail){sent.push(p);pending.push({ok:ok,fail:fail});},openURL:function(u){assert(u.indexOf('data:')===0);}}};
vm.runInNewContext(fs.readFileSync(require.resolve('../src/pkjs/index'),'utf8'),context);
events.ready();assert.equal(sent[0].ShowBtIcon,1);assert.equal(sent[0].BtVibrate,0);assert.equal(sent[0].LowVisionMode,0);pending.shift().ok();assert.equal(geo,1);
events.webviewclosed({response:JSON.stringify({TemperatureUnit:{value:'c'},ShowBtIcon:{value:false},BtVibrate:{value:true},LowVisionMode:{value:true}})});
assert.equal(sent[1].TemperatureUnit,1);assert.equal(sent[1].ShowBtIcon,0);assert.equal(sent[1].BtVibrate,1);assert.equal(sent[1].LowVisionMode,1);
pending.shift().fail();timers.shift()();assert.equal(sent.length,3);pending.shift().ok();
events.webviewclosed({response:'CANCELLED'});assert.equal(sent.length,3);
events.showConfiguration();events.appmessage({payload:{RequestWeather:1}});
assert.equal(geo,1); // failed fetch backoff
var live=process.argv[2];
if(live) {
 var response=JSON.parse(fs.readFileSync(live,'utf8'));
 var c=response.current;
 assert(f.engine.valid({TempC:Math.round(c.temperature_2m*100),WeatherCode:c.weather_code,IsDay:c.is_day,WeatherStamp:c.time}));
 console.log('Live Open-Meteo response schema accepted');
}
console.log('Companion: weather success/cache/expiry/failure/backoff, settings, queue and retry passed');
