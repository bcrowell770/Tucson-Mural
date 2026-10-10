var Clay=require('@rebble/clay');
var clay=new Clay(require('./config'),null,{autoHandleEvents:false});
var queue=[],sending=false;
function send(payload) {
  queue.push({payload:payload,tries:0});
  pump();
}
function pump() {
  if(sending||!queue.length) return;
  sending=true;
  var item=queue[0];
  Pebble.sendAppMessage(item.payload,function() {queue.shift();sending=false;pump();},function() {
    sending=false;
    if(++item.tries>=3) {queue.shift();pump();}
    else {sending=true;setTimeout(function(){sending=false;pump();},1500);}
  });
}
var weather=require('./weather')({
  now:function(){return Math.floor(Date.now()/1000);},storage:localStorage,send:send,
  log:function(s){console.log(s);},
  locate:function(ok,fail){navigator.geolocation.getCurrentPosition(ok,fail,{timeout:10000,maximumAge:1800000});},
  get:function(url,ok,fail) {
    var xhr=new XMLHttpRequest(),done=false;
    function failure(){if(!done){done=true;fail();}}
    xhr.open('GET',url,true);xhr.timeout=15000;
    xhr.onload=function(){if(done)return;done=true;if(xhr.status!==200){fail();return;}
      try{ok(JSON.parse(xhr.responseText));}catch(e){fail();}};
    xhr.onerror=failure;xhr.ontimeout=failure;
    try{xhr.send();}catch(e){failure();}
  }
});
function settings() {
  try {return JSON.parse(localStorage.getItem('tucson.settings.v1'))||{};}catch(e){return {};}
}
function value(s,key,fallback) {
  var v=s[key];if(v&&typeof v==='object')v=v.value;
  return typeof v==='undefined'?fallback:v;
}
function publishSettings(s) {
  send({TemperatureUnit:value(s,'TemperatureUnit','f')==='c'?1:0,
    ShowBtIcon:value(s,'ShowBtIcon',true)?1:0,BtVibrate:value(s,'BtVibrate',false)?1:0,LowVisionMode:value(s,'LowVisionMode',false)?1:0});
}
Pebble.addEventListener('ready',function(){publishSettings(settings());weather.refresh(false);});
Pebble.addEventListener('appmessage',function(e){if(e.payload.RequestWeather)weather.refresh(false);});
Pebble.addEventListener('showConfiguration',function(){Pebble.openURL(clay.generateUrl());});
Pebble.addEventListener('webviewclosed',function(e){
  if(!e.response||e.response==='CANCELLED')return;
  try {
    var s=clay.getSettings(e.response,false);
    localStorage.setItem('tucson.settings.v1',JSON.stringify(s));publishSettings(s);
  } catch(error){console.log('Settings could not be read: '+error);}
});
