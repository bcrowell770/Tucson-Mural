from PIL import Image, ImageDraw

W,H=32,112

def glyph(n, size=None):
    if n in [3,6,9]:
        return block_glyph(n,size or (W,H))
    m = Image.new('L', (W, H))
    d = ImageDraw.Draw(m)
    if n == 1:
        d.polygon([(3, 18), (18, 0), (29, 0), (29, 111), (9, 111), (9, 35), (3, 39)], fill=255)
    if n == 0:
        d.rounded_rectangle((0, 0, 31, 111), radius=13, fill=255)
        d.rounded_rectangle((12, 23, 19, 88), radius=4, fill=0)
    if n == 4:
        d.polygon([(17, 0), (30, 0), (30, 72), (31, 72), (31, 91), (30, 91), (30, 111), (13, 111), (13, 91), (0, 91), (0, 70)], fill=255)
        d.polygon([(13, 34), (13, 72), (5, 72)], fill=0)
    if n == 2:
        d.polygon([(0, 29), (0, 14), (3, 6), (10, 0), (22, 0), (29, 5), (31, 13), (31, 35), (28, 47), (12, 91), (31, 91), (31, 111), (0, 111), (0, 92), (18, 42), (20, 33), (20, 25), (18, 21), (13, 21), (11, 25), (11, 29)], fill=255)
    if n == 5:
        d.polygon([(0,0),(31,0),(31,20),(12,20),(12,43),(22,43),(31,52),(31,98),(23,111),(0,111),(0,91),(18,91),(20,86),(20,68),(17,63),(0,63)],fill=255)
    if n == 7:
        d.polygon([(0,0),(31,0),(31,22),(13,111),(0,111),(18,23),(0,23)],fill=255)
    if n == 8:
        d.rounded_rectangle((0,0,31,111),radius=12,fill=255)
        d.rounded_rectangle((12,20,19,42),radius=3,fill=0)
        d.rounded_rectangle((12,68,19,91),radius=3,fill=0)
        d.polygon([(0,43),(4,51),(4,60),(0,68)],fill=0)
        d.polygon([(31,43),(27,51),(27,60),(31,68)],fill=0)
    return m

# Shared block-letter grammar: 20-unit horizontal bars, 11–12-unit
# vertical stems and small chamfers. Draw directly at native resolution.
def block_glyph(n,size):
    w,h=size
    m=Image.new("L",(w*4,h*4))
    d=ImageDraw.Draw(m)
    def poly(points,fill):
        d.polygon([(round(x*w*4/32),round(y*h*4/112)) for x,y in points],fill=fill)
    if n==3:
        poly([(3,0),(24,0),(32,8),(32,43),(27,55),(32,67),
              (32,104),(24,112),(3,112),(0,109),(0,92),
              (17,92),(20,89),(20,69),(17,66),(6,66),
              (6,46),(17,46),(20,43),(20,23),(17,20),(0,20),(0,3)],255)
    elif n==6:
        poly([(13,0),(32,0),(32,20),(18,20),(12,28),(12,46),
              (24,46),(32,54),(32,102),(23,112),(9,112),
              (0,103),(0,24)],255)
        poly([(14,66),(19,66),(21,68),(21,90),(19,92),
              (14,92),(12,90),(12,68)],0)
    elif n==9:
        poly([(8,0),(24,0),(32,8),(32,87),(18,112),(3,112),
              (3,92),(11,92),(20,79),(20,64),(8,64),(0,56),(0,8)],255)
        poly([(14,20),(19,20),(21,22),(21,43),(19,45),
              (14,45),(12,43),(12,22)],0)
    return m.resize(size,Image.Resampling.LANCZOS).point(lambda v:255 if v>=128 else 0)
