const C=require('./core.js'),fs=require('fs'),path=require('path');
const root='/home/claude/game/Data/Mission';let ok=0,bad=0,sok=0,sbad=0;
function walk(d){for(const f of fs.readdirSync(d)){const p=path.join(d,f);if(fs.statSync(p).isDirectory())walk(p);else{
 const b=fs.readFileSync(p);
 if(/\.map$/i.test(f)){try{const m=C.parseMap(b);const o=Buffer.from(C.buildMap(m));if(o.equals(b))ok++;else{bad++;console.log('MAP DIFF',p,o.length,b.length)}}catch(e){bad++;console.log('MAP ERR',p,e.message)}}
 if(/\.stg$/i.test(f)&&b.readInt32LE(0)===0x3e9){try{const s=C.parseStg(b);const o=Buffer.from(C.buildStg(s));if(o.equals(b))sok++;else{sbad++;console.log('STG DIFF',p)}}catch(e){sbad++;console.log('STG ERR',p,e.message)}}
}}}
walk(root);console.log('maps ok',ok,'bad',bad,'stg ok',sok,'bad',sbad);
const d=C.decodeDDS(fs.readFileSync(root+'/Live/9_IceStorm.dds'));const e=C.encodeDDS(d.rgba,d.w,d.h);const d2=C.decodeDDS(e);
let err=0;for(let i=0;i<d.rgba.length;i++)err+=Math.abs(d.rgba[i]-d2.rgba[i]);console.log('dds',d.w,d.h,e.length,'mean err',(err/d.rgba.length).toFixed(2));
const m=C.parseMap(fs.readFileSync(root+'/Live/9_IceStorm.map'));let mx=0;for(const p of m.props){mx=Math.max(mx,Math.abs(C.groundHeight(m,p.x,p.y)-p.h))}console.log('max prop height diff',mx);
const z=C.makeZip([{name:'a.txt',data:Buffer.from('hello')},{name:'b.bin',data:e}]);fs.writeFileSync('t.zip',z);
const n=C.newMap(200,1,300);const nb=C.buildMap(n);const n2=C.parseMap(nb);console.log('newmap',nb.length,n2.W,n2.tileset);
