"""Native-resolution exports, shared lighting roles, packed 4-bit backgrounds."""
from pathlib import Path
import json
from PIL import Image, ImageFilter, ImageOps
from glyph_geometry import glyph
from prepare_lighting import packed, is_sky

ROOT=Path(__file__).resolve().parents[1]
PROFILES={'basalt':(144,168,32,112),'emery':(200,228,44,152),'gabbro':(260,260,46,144)}
THEMES=['morning','day','sunset','night']
DIGITS=list(range(10))
PALETTES={'morning':[(255,255,255),(255,255,170)],
 'day':[(255,255,255),(170,255,255),(170,170,255)],
 'sunset':[(170,85,170),(170,85,85),(255,170,170),(255,170,85),(255,255,170)]}
atlas=Image.open(ROOT/'art/numeral-scene-atlas.png').convert('RGB')
media=[]
for platform,(w,h,tw,th) in PROFILES.items():
 out=ROOT/'resources'/platform; out.mkdir(exist_ok=True)
 for panel,n in enumerate(DIGITS):
  if platform=='basalt' and n in [1,0,4,2]:
   # Preserve the reviewed Basalt pixels, including their original outline.
   raw=bytearray((ROOT/'resources/tiles'/f'digit_{n}_v0.bin').read_bytes())
  else:
   if n in [1,0,4,2]:
    panel=[1,0,4,2].index(n)
    source=atlas.crop((panel*335,0,(panel+1)*335,1174))
   else:
    source=Image.open(ROOT/'art/scene-9-quail-source.png' if n==9 else ROOT/f'art/scene-{n}-v2-source.png' if n in [3,6] else ROOT/f'art/scene-{n}-source.png').convert('RGB')
    # Frame the hummingbird's face in the 7's top bar; the diagonal keeps its body.
    if n==7: source=source.crop((0,round(source.height*.18),source.width,source.height))
   scene=source.resize((tw,th),Image.Resampling.LANCZOS)
   if n==1:
    narrow=atlas.crop((0,0,335,1174)).resize((round(tw*22/32),th),Image.Resampling.LANCZOS)
    scene.paste(narrow,(round(tw*7/32),0))
   scene=scene.filter(ImageFilter.GaussianBlur(.3 if n==1 else .7))
   mask=glyph(n,(tw,th)) if n in [3,6,9] else glyph(n).resize((tw,th),Image.Resampling.LANCZOS).point(lambda v:255 if v>=128 else 0)
   inner=ImageOps.expand(mask,1).filter(ImageFilter.MinFilter(3)).crop((1,1,tw+1,th+1))
   raw=bytearray()
   for y in range(th):
    for x in range(tw):
     if not mask.getpixel((x,y)): raw.append(0)
     elif not inner.getpixel((x,y)): raw.append(0x40)
     else:
      c=packed(scene.getpixel((x,y)))
      rgb=(((c>>4)&3)*85,((c>>2)&3)*85,(c&3)*85)
      raw.append((0x80 if is_sky(n,x*32/tw,y*112/th,rgb) else 0xc0)|(c&63))
  name=f'DIGIT_{n}_{platform.upper()}'
  (out/f'digit-{n}.bin').write_bytes(raw)
  media.append({'type':'raw','name':name,'file':f'{platform}/digit-{n}.bin','targetPlatforms':[platform]})
 for theme in THEMES:
  source=Image.open(ROOT/f'art/background-{theme}-source.png').convert('RGB')
  if theme=='night': source=source.filter(ImageFilter.MaxFilter(9))
  native=source.resize((w,h),Image.Resampling.LANCZOS).filter(ImageFilter.GaussianBlur(.7))
  if theme=='night': native=native.point(lambda v:min(255,((v+42)//85)*85))
  else:
   colors=PALETTES[theme]; palette=Image.new('P',(1,1))
   palette.putpalette([v for c in colors for v in c]+list(colors[0])*(256-len(colors)))
   native=native.quantize(palette=palette,dither=Image.Dither.FLOYDSTEINBERG).convert('RGB')
  values=[packed(c) for c in native.getdata()]
  colors=sorted(set(values)); assert len(colors)<=16,(theme,len(colors))
  indices={c:i for i,c in enumerate(colors)}
  data=bytearray(colors+[0xc0]*(16-len(colors)))
  for i in range(0,len(values),2): data.append(indices[values[i]]<<4|indices[values[i+1]])
  (out/f'{theme}.bin').write_bytes(data)
  media.append({'type':'raw','name':f'BACKGROUND_{theme.upper()}_{platform.upper()}',
                'file':f'{platform}/{theme}.bin','targetPlatforms':[platform]})
 print(platform, 'raw resources',sum(p.stat().st_size for p in out.glob('*.bin')),
       'image pixels RAM',w*h//2+4*tw*th,'all ten projected',w*h//2*4+64+10*tw*th)
manifest=json.loads((ROOT/'package.json').read_text())
manifest['pebble']['targetPlatforms']=['emery','gabbro','basalt']
manifest['pebble']['resources']['media']=media
(ROOT/'package.json').write_text(json.dumps(manifest,indent=2)+'\n')
