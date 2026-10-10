module.exports = [
  {type:'heading', defaultValue:'Tucson Mural'},
  {type:'section', items:[
    {type:'heading', defaultValue:'Weather'},
    {type:'select', messageKey:'TemperatureUnit', label:'Temperature unit', defaultValue:'f', options:[
      {label:'Fahrenheit',value:'f'}, {label:'Celsius',value:'c'}]},
    {type:'text', defaultValue:'Uses your phone location for weather from Open-Meteo. Updates every 30 minutes. A small dot beneath the weather icon marks data over an hour old; after two hours, -- is shown.'},
    {type:'text', defaultValue:'Weather: <a href="https://open-meteo.com/">Open-Meteo</a>'}
  ]},
  {type:'section', items:[
    {type:'heading', defaultValue:'Bluetooth'},
    {type:'toggle', messageKey:'ShowBtIcon', label:'Show icon when disconnected', defaultValue:true},
    {type:'toggle', messageKey:'BtVibrate', label:'Vibrate on disconnect', defaultValue:false},
    {type:'text', defaultValue:'One short pulse when the phone disconnects. No startup or repeated alerts.'}
  ]},
  {type:'section', items:[
    {type:'heading', defaultValue:'Accessibility'},
    {type:'toggle', messageKey:'LowVisionMode', label:'Low Vision Mode', defaultValue:false},
    {type:'text', defaultValue:'Larger date and weather. Long dates shorten to month/day when needed. Mural numerals and backgrounds stay unchanged.'}
  ]},
  {type:'submit', defaultValue:'Save settings'}
];

