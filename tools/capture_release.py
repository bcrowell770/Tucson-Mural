import time,datetime,subprocess,json,os,argparse,shutil
from pathlib import Path
from uuid import UUID
from libpebble2.communication import PebbleConnection
from pebble_tool.sdk.emulator import ManagedEmulatorTransport
from libpebble2.protocol.system import TimeMessage,SetUTC
from libpebble2.protocol.logs import AppLogMessage,AppLogShippingControl
from libpebble2.services.appmessage import AppMessageService,Int32
from libpebble2.services.screenshot import Screenshot
from libpebble2.communication.transports.websocket import MessageTargetPhone
from libpebble2.communication.transports.websocket.protocol import WebSocketPhonesimAppConfig,AppConfigResponse,AppConfigSetup,WebSocketPhonesimConfigResponse
from pebble_tool.commands.emucontrol import send_data_to_qemu
from libpebble2.communication.transports.qemu.protocol import QemuTimeFormat
from PIL import Image
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description='Capture controlled review fixtures from Pebble SDK emulators.')
parser.add_argument('--pbw',type=Path,default=root/'releases/tucson-mural-1.0.0.pbw')
parser.add_argument('--output',type=Path,default=root/'build/capture-review')
args=parser.parse_args();pbw_path=args.pbw.resolve();pkg=args.output.resolve()
for platform in ['basalt','emery','gabbro']:(pkg/'screenshots'/platform).mkdir(parents=True,exist_ok=True)
(pkg/'evidence').mkdir(parents=True,exist_ok=True)
pebble=shutil.which('pebble')
if not pebble:raise SystemExit('Pebble CLI is required on PATH.')
os.chdir(root)
uuid=UUID('dba27ea0-aebb-4653-82f7-2c52e9ecb29b')
scenes=[('01-morning',7,15,2000,1,False),('02-day',10,42,2550,1,False),('03-sunset',18,36,2720,1,False),('04-night',20,59,1800,0,False),('05-low-vision',12,47,2550,1,True)]
for platform in ['basalt','emery','gabbro']:
 r=subprocess.run([pebble,'install','--emulator',platform,str(pbw_path)],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=50)
 assert r.returncode==0,r.stdout
 pb=PebbleConnection(ManagedEmulatorTransport(platform,'4.17'));pb.connect();pb.run_async()
 logs=[]
 pb.register_endpoint(AppLogMessage,lambda p,store=logs:store.append(str(p.message)))
 pb.send_packet(AppLogShippingControl(enable=True));app=AppMessageService(pb);time.sleep(5)
 send_data_to_qemu(pb.transport,QemuTimeFormat(is_24_hour=True));time.sleep(.3)
 def settings(low):
  pb.transport.send_packet(WebSocketPhonesimAppConfig(config=AppConfigSetup()),target=MessageTargetPhone())
  pb.read_transport_message(MessageTargetPhone,WebSocketPhonesimConfigResponse)
  pb.transport.send_packet(WebSocketPhonesimAppConfig(config=AppConfigResponse(data=json.dumps({'TemperatureUnit':'f','ShowBtIcon':True,'BtVibrate':False,'LowVisionMode':low}))),target=MessageTargetPhone());time.sleep(.7)
 for name,hour,minute,temp,day,low in scenes:
  settings(low);start=len(logs)
  stamp=int(datetime.datetime(2026,10,1,hour,minute,tzinfo=datetime.timezone.utc).timestamp())
  for t in [stamp-61,stamp-1]:
   pb.send_packet(TimeMessage(message=SetUTC(unix_time=t,utc_offset=0,tz_name='UTC')));time.sleep(2)
  deadline=time.monotonic()+15
  expected=f'{hour:02d}:{minute:02d} display'
  while not any(expected in s for s in logs[start:]) and time.monotonic()<deadline:time.sleep(.2)
  assert any(expected in s for s in logs[start:]),logs[start:]
  app.send_message(uuid,{k:Int32(v) for k,v in {1:temp,2:0,3:day,4:stamp}.items()});time.sleep(.7)
  rows=Screenshot(pb).grab_image();h=len(rows);w=len(rows[0])//3
  im=Image.frombytes('RGB',(w,h),bytes(v for row in rows for v in row))
  assert (w,h)=={'basalt':(144,168),'emery':(200,228),'gabbro':(260,260)}[platform]
  im.save(pkg/'screenshots'/platform/(name+'.png'))
  print(platform,name,flush=True)
 settings(False)
 pb.send_packet(TimeMessage(message=SetUTC(unix_time=int(time.time()),utc_offset=0,tz_name='UTC')))
 (pkg/'evidence'/f'{platform}-capture.log').write_text('\n'.join(logs)+'\n')
print('15 native captures complete',flush=True)
