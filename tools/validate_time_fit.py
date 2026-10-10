"""Exhaustively check actual asset silhouettes for all 1,440 24-hour times.

Uses the C layout profiles, exported raw tile alpha/outline data, and an SDK
emulator-derived circular screen fixture. Colon boxes conservatively enclose
both the day circles and night crescent/star foreground. One empty pixel
column is required between neighbouring components on every shared row.
"""
from pathlib import Path
import json,re,argparse

ROOT=Path(__file__).resolve().parents[1]

def validate(root):
    source=(root/'src/c/layout_profile.h').read_text()
    result={}
    for platform in ['emery','gabbro','basalt']:
        block=re.search(r'(?:#if|#elif) defined\(PBL_PLATFORM_'+platform.upper()+r'\)(.*?)(?=#elif|#else)',source,re.S)[1]
        d={k:int(v) for k,v in re.findall(r'#define PROFILE_(\w+) (\d+)',block)}
        xs=list(map(int,re.search(r'TILE_X\[4\]=\{([^}]+)',block)[1].split(',')))
        w,h,tw,th,top=[d[k] for k in ['WIDTH','HEIGHT','TILE_WIDTH','TILE_HEIGHT','TILE_TOP']]
        fixture=root/'tools/gabbro-visible-rows.json'
        if not fixture.exists():fixture=ROOT/'tools/gabbro-visible-rows.json'
        visible=json.loads(fixture.read_text())['rows'] if platform=='gabbro' else [[0,w-1]]*h
        assert len(visible)==h
        masks={}
        for digit in range(10):
            data=(root/f'resources/{platform}/digit-{digit}.bin').read_bytes()
            assert len(data)==tw*th,(platform,digit,'tile size')
            masks[digit]=[]
            for y in range(th):
                ink=[x for x in range(tw) if data[y*tw+x]!=0]
                if ink: masks[digit].append((y+top,min(ink),max(ink)))
        colon={}
        radius=max(d['COLON_RADIUS'],3) # fixed seven-pixel night star
        for cy in [d['COLON_Y'],d['COLON_Y']+d['COLON_STEP']]:
            for y in range(cy-radius,cy+radius+1):
                colon[y]=(d['COLON_X']-radius,d['COLON_X']+radius)
        report={'times_checked':0,'max_ink_width':0,'widest_example':None,
                'widest_tie_count':0,'min_screen_clearance_px':w,
                'edge_example':None,'min_component_gap_px':w,'gap_example':None,
                'fixed_tile_row_width_px':xs[-1]+tw-xs[0],
                'tile_row_left_inset_px':xs[0],'tile_row_right_inset_px':w-(xs[-1]+tw)}
        failures=[]
        for hour in range(24):
            for minute in range(60):
                time=f'{hour:02d}:{minute:02d}'
                digits=[hour//10,hour%10,minute//10,minute%10]
                rows={y:[('colon',a,b)] for y,(a,b) in colon.items()}
                for slot,digit in enumerate(digits):
                    for y,a,b in masks[digit]:
                        rows.setdefault(y,[]).append((f'digit{slot}:{digit}',a+xs[slot],b+xs[slot]))
                left=min(a for parts in rows.values() for _,a,b in parts)
                right=max(b for parts in rows.values() for _,a,b in parts)
                width=right-left+1
                if width>report['max_ink_width']:
                    report.update(max_ink_width=width,widest_example=time,widest_tie_count=1)
                elif width==report['max_ink_width']:report['widest_tie_count']+=1
                for y,parts in rows.items():
                    if not 0<=y<h:
                        failures.append([time,'vertical clipping',y]);continue
                    lo,hi=visible[y]
                    for name,a,b in parts:
                        clearance=min(a-lo,hi-b)
                        if clearance<report['min_screen_clearance_px']:
                            report.update(min_screen_clearance_px=clearance,edge_example={'time':time,'row':y,'component':name})
                        if clearance<0:failures.append([time,'screen clipping',y,name])
                    parts.sort(key=lambda part:part[1])
                    for first,second in zip(parts,parts[1:]):
                        gap=second[1]-first[2]-1
                        if gap<report['min_component_gap_px']:
                            report.update(min_component_gap_px=gap,gap_example={'time':time,'row':y,'between':[first[0],second[0]]})
                        if gap<1:failures.append([time,'less than one empty column',y,first[0],second[0]])
                report['times_checked']+=1
        report['passed']=not failures
        report['failure_count']=len(failures)
        report['first_failures']=failures[:5]
        result[platform]=report
    return result

if __name__=='__main__':
    p=argparse.ArgumentParser()
    p.add_argument('--root',type=Path,default=ROOT)
    p.add_argument('--output',type=Path)
    args=p.parse_args()
    results=validate(args.root)
    if args.output:args.output.write_text(json.dumps(results,indent=2)+'\n')
    for platform,r in results.items():
        print(platform, 'PASS' if r['passed'] else 'FAIL',r['times_checked'],'times;',
              'max ink width',r['max_ink_width'],'at',r['widest_example'],
              '; minimum edge clearance',r['min_screen_clearance_px'],
              '; minimum component gap',r['min_component_gap_px'])
        if not r['passed']:print(r['first_failures'])
    raise SystemExit(0 if all(r['passed'] for r in results.values()) else 1)
