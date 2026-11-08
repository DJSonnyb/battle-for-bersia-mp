"""Fire at Will ability icon, 36x36, painted style like the shipped ability icons (binary alpha)."""
import os as _os_; ROOT = _os_.path.dirname(_os_.path.dirname(_os_.path.dirname(_os_.path.abspath(__file__)))) + '/'

from PIL import Image, ImageDraw, ImageFilter
K=8; W=36
big=Image.new('RGBA',(W*K,W*K),(0,0,0,0)); d=ImageDraw.Draw(big)
def P(x,y): return (x*K,y*K)
# target: red / white rings, lower right
cx,cy=21,22
for r,col in ((11,(150,20,20)),(9.5,(235,60,40)),(7,(250,240,225)),(5,(220,40,30)),(2.6,(255,235,90))):
    d.ellipse([*P(cx-r,cy-r),*P(cx+r,cy+r)],fill=col+(255,))
# three arrows coming from upper left into the target
def arrow(x0,y0,x1,y1):
    d.line([*P(x0,y0),*P(x1,y1)],fill=(70,50,30,255),width=int(1.6*K))
    d.line([*P(x0,y0),*P(x1,y1)],fill=(150,110,60,255),width=int(0.8*K))
    # head
    import math
    a=math.atan2(y1-y0,x1-x0); hl=3.2; hw=1.7
    tip=(x1+math.cos(a)*1.5,y1+math.sin(a)*1.5)
    b1=(x1-math.cos(a)*hl+math.sin(a)*hw, y1-math.sin(a)*hl-math.cos(a)*hw)
    b2=(x1-math.cos(a)*hl-math.sin(a)*hw, y1-math.sin(a)*hl+math.cos(a)*hw)
    d.polygon([P(*tip),P(*b1),P(*b2)],fill=(200,215,235,255),outline=(40,50,70,255))
    # fletching
    for s in (1,-1):
        f1=(x0+math.sin(a)*1.8*s, y0-math.cos(a)*1.8*s); f2=(x0+math.cos(a)*3,y0+math.sin(a)*3)
        d.polygon([P(x0,y0),P(*f1),P(*f2)],fill=(240,240,240,255))
arrow(3,4,16,17); arrow(9,2,20,15); arrow(2,11,15,20)
# dark outline glow like the game icons
a=big.split()[3].filter(ImageFilter.MaxFilter(K*2+1))
glow=Image.new('RGBA',big.size,(25,25,25,255)); glow.putalpha(a)
out=Image.alpha_composite(glow,big).resize((W,W),Image.LANCZOS)
px=out.load()
for y in range(W):
    for x in range(W):
        r,g,b,al=px[x,y]; px[x,y]=(r,g,b,255) if al>=110 else (0,0,0,0)
out.save(ROOT+'jobs/icons/FireAtWill.png')
out.resize((144,144),Image.NEAREST).save(ROOT+'jobs/abil/FireAtWill_x4.png')
