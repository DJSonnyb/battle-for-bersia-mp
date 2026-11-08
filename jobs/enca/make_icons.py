"""Encablossa job icons (42x41, game style): Dwarf.png frame, black interior, white symbol drawn at 8x and thresholded."""
import os as _os_; ROOT = _os_.path.dirname(_os_.path.dirname(_os_.path.dirname(_os_.path.abspath(__file__)))) + '/'

from PIL import Image, ImageDraw
import math
S=ROOT+'jobs/'
K=8
def base(gold=False):
    im=Image.open(S+'icons/Dwarf.png').convert('RGBA'); px=im.load()
    for y in range(41):
        for x in range(42):
            r,g,b,a=px[x,y]
            if a and abs(x-20.5)+abs(y-20)<16.5: px[x,y]=(0,0,0,255)
            elif gold and a and g>120 and g>r: px[x,y]=(min(255,g),int(g*0.82),int(b*0.45),a)
    return im
def finish(name,draw_fn,gold=False):
    big=Image.new('L',(42*K,41*K),0); d=ImageDraw.Draw(big); draw_fn(d)
    m=big.resize((42,41),Image.BOX)
    im=base(gold); px=im.load(); mp=m.load()
    for y in range(41):
        for x in range(42):
            if mp[x,y]>110 and abs(x-20.5)+abs(y-20)<15.5: px[x,y]=(255,251,255,255)
    im.save(S+'icons/%s.png'%name)
    im.resize((168,164),Image.NEAREST).save(S+'enca/%s_x4.png'%name)
def P(x,y): return (x*K,y*K)
def brood(d):   # spiky carapace, three eyes, big hooked mandibles
    d.ellipse([*P(13,12),*P(29,24)],fill=255)
    for ex,ey in ((17,16),(25,16),(21,19)): d.ellipse([*P(ex-1.5,ey-1.5),*P(ex+1.5,ey+1.5)],fill=0)
    d.polygon([P(14,21),P(12,29),P(15,31),P(15.5,27),P(19,23)],fill=255)
    d.polygon([P(28,21),P(30,29),P(27,31),P(26.5,27),P(23,23)],fill=255)
    for sx,sy in ((15,13),(21,11),(27,13)): d.polygon([P(sx-2,sy+1),P(sx,sy-4),P(sx+2,sy+1)],fill=255)
def spitter(d): # small head lower left, three droplets arcing up right
    d.ellipse([*P(10,19),*P(22,30)],fill=255)
    d.ellipse([*P(13,22),*P(15.5,24.5)],fill=0)
    d.polygon([P(19,22),P(24,20),P(20,26)],fill=255)
    for (cx,cy,r) in ((25,18,2.2),(28.5,13.5,2.6),(31,9.5,1.8)):
        d.ellipse([*P(cx-r,cy-r),*P(cx+r,cy+r)],fill=255)
def infested(d):# lumpy swollen body covered in eyes, spores rising
    for (cx,cy,r) in ((21,22,7),(16,25,4.5),(26,25,4.5),(21,16,4.5)): d.ellipse([*P(cx-r,cy-r),*P(cx+r,cy+r)],fill=255)
    for (cx,cy) in ((18,21),(24,21),(21,25),(16.5,26),(25.5,26),(21,16.5)): d.ellipse([*P(cx-1.1,cy-1.1),*P(cx+1.1,cy+1.1)],fill=0)
    for (cx,cy,r) in ((14,14,1.4),(28,14,1.4),(21,9,1.6)): d.ellipse([*P(cx-r,cy-r),*P(cx+r,cy+r)],fill=255)
def flyer(d):   # insect body with spread wings
    d.ellipse([*P(19,11),*P(23,30)],fill=255)
    d.polygon([P(20,16),P(9,11),P(8,17),P(13,21),P(20,20)],fill=255)
    d.polygon([P(22,16),P(33,11),P(34,17),P(29,21),P(22,20)],fill=255)
    d.line([*P(20,11.5),*P(17,8)],fill=255,width=K); d.line([*P(22,11.5),*P(25,8)],fill=255,width=K)
finish('Enca_Brood',brood); finish('Enca_Spitter',spitter); finish('Enca_Infested',infested); finish('Enca_Flyer',flyer,gold=True)
