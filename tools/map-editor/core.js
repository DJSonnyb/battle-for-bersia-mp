/* ===== Battle for Bersia map format core (no DOM) ===== */
const TILESETS = ['area01_maptile.tga','area02_maptile.tga','area03_maptile.tga','area04_maptile.tga','area05_maptile.tga','area06_maptile.tga','world_maptile.tga','area07_maptile.tga','area08_maptile.tga'];
const TILESET_NAMES = ['Desert','Grass','Dark grass','Mud','Snow','Inside Encablossa','World map','Forest','Lava'];
const CELL = 125;

function tilesetIndex(name) { const n = name.toLowerCase(); for (let i = 0; i < TILESETS.length; i++) if (n.startsWith(TILESETS[i].slice(0, 6))) return i; return -1; }

/* --- .map --- */
function parseMap(buf) {
  const u8 = new Uint8Array(buf), dv = new DataView(u8.buffer, u8.byteOffset, u8.byteLength);
  if (u8.length < 16 || dv.getUint32(0, true) !== 0x6d) throw new Error('This is not a Kingdom Under Fire map file (version 109 expected).');
  const N = dv.getUint32(4, true);
  if (N < 1 || N > 16) throw new Error('Map file has an unexpected chunk count (' + N + ').');
  const dir = [];
  for (let i = 0; i < N; i++) dir.push({ id: dv.getUint32(8 + i * 8, true), off: dv.getUint32(12 + i * 8, true) });
  const sorted = dir.slice().sort((a, b) => a.off - b.off);
  sorted.forEach((d, i) => { d.end = i + 1 < sorted.length ? sorted[i + 1].off : u8.length; });
  const raw = {};
  for (const d of dir) raw[d.id] = u8.slice(d.off, d.end);
  if (!raw[0] || !raw[1] || !raw[2]) throw new Error('Map file is missing its terrain, prop or cell chunk.');
  const c0 = raw[0], c0v = new DataView(c0.buffer);
  let name = ''; for (let i = 0; i < 256 && c0[i]; i++) name += String.fromCharCode(c0[i]);
  const W = c0v.getUint32(256, true), H = c0v.getUint32(260, true);
  if (W < 8 || W > 1024 || H < 8 || H > 1024) throw new Error('Map size ' + W + 'x' + H + ' is not supported.');
  const tiles = new Uint16Array(W * H), heights = new Int16Array((W + 1) * (H + 1));
  for (let i = 0; i < W * H; i++) tiles[i] = c0v.getUint16(264 + i * 2, true);
  const hb = 264 + 2 * W * H;
  for (let i = 0; i < heights.length; i++) heights[i] = c0v.getInt16(hb + i * 2, true);
  const c1v = new DataView(raw[1].buffer), np = c1v.getUint32(0, true), props = [];
  for (let i = 0; i < np; i++) {
    const o = 4 + i * 24;
    props.push({ id: c1v.getInt32(o, true), inst: c1v.getInt32(o + 4, true), x: c1v.getFloat32(o + 8, true), h: c1v.getFloat32(o + 12, true), y: c1v.getFloat32(o + 16, true), rot: c1v.getUint32(o + 20, true), orig: i });
  }
  const c2v = new DataView(raw[2].buffer), aw = c2v.getUint32(0, true), ah = c2v.getUint32(4, true);
  const attr = raw[2].slice(8, 8 + aw * ah);
  const grid = id => { if (!raw[id]) return null; const v = new DataView(raw[id].buffer); const w = v.getUint32(0, true), h = v.getUint32(4, true); return { w, h, data: raw[id].slice(8, 8 + w * h) }; };
  return { order: dir.map(d => d.id), raw, tileset: name, W, H, tiles, heights, props, attr, aw, ah, open: grid(3), fuel: grid(5) };
}

function buildMap(m) {
  const W = m.W, H = m.H;
  const chunks = {};
  for (const id of m.order) chunks[id] = m.raw[id];
  /* chunk 0 */
  const c0 = new Uint8Array(264 + 2 * W * H + 2 * (W + 1) * (H + 1)), c0v = new DataView(c0.buffer);
  for (let i = 0; i < m.tileset.length && i < 255; i++) c0[i] = m.tileset.charCodeAt(i);
  if (m.raw[0] && !m.tilesetChanged) for (let i = m.tileset.length; i < 256; i++) c0[i] = m.raw[0][i];   /* keep the original padding bytes */
  c0v.setUint32(256, W, true); c0v.setUint32(260, H, true);
  for (let i = 0; i < W * H; i++) c0v.setUint16(264 + i * 2, m.tiles[i], true);
  const hb = 264 + 2 * W * H;
  for (let i = 0; i < m.heights.length; i++) c0v.setInt16(hb + i * 2, m.heights[i], true);
  chunks[0] = c0;
  /* chunk 1 */
  const c1 = new Uint8Array(4 + 24 * m.props.length), c1v = new DataView(c1.buffer);
  c1v.setUint32(0, m.props.length, true);
  m.props.forEach((p, i) => { const o = 4 + i * 24; c1v.setInt32(o, p.id, true); c1v.setInt32(o + 4, p.inst, true); c1v.setFloat32(o + 8, p.x, true); c1v.setFloat32(o + 12, p.h, true); c1v.setFloat32(o + 16, p.y, true); c1v.setUint32(o + 20, p.rot >>> 0, true); });
  chunks[1] = c1;
  /* chunk 2 */
  const c2 = new Uint8Array(8 + m.attr.length), c2v = new DataView(c2.buffer);
  c2v.setUint32(0, m.aw, true); c2v.setUint32(4, m.ah, true); c2.set(m.attr, 8);
  chunks[2] = c2;
  const g = (o) => { const c = new Uint8Array(8 + o.w * o.h), v = new DataView(c.buffer); v.setUint32(0, o.w, true); v.setUint32(4, o.h, true); c.set(o.data, 8); return c; };
  if (m.open) chunks[3] = g(m.open);
  if (m.fuel) chunks[5] = g(m.fuel);
  /* PINF: distinct prop ids in first-use order */
  const seen = new Set(), pinf = [];
  for (const p of m.props) if (!seen.has(p.id)) { seen.add(p.id); pinf.push(p.id); }
  const order = m.order;
  const hdr = 8 + 8 * order.length + 8 + 4 * pinf.length;
  let tot = hdr; for (const id of order) tot += chunks[id].length;
  const out = new Uint8Array(tot), ov = new DataView(out.buffer);
  ov.setUint32(0, 0x6d, true); ov.setUint32(4, order.length, true);
  let off = hdr;
  order.forEach((id, i) => { ov.setUint32(8 + i * 8, id, true); ov.setUint32(12 + i * 8, off, true); out.set(chunks[id], off); off += chunks[id].length; });
  const pp = 8 + 8 * order.length;
  out[pp] = 0x50; out[pp + 1] = 0x49; out[pp + 2] = 0x4e; out[pp + 3] = 0x46;
  ov.setUint32(pp + 4, pinf.length, true);
  pinf.forEach((id, i) => ov.setInt32(pp + 8 + i * 4, id, true));
  return out;
}

/* new map from nothing (the way the mod's Plains map is made) */
function newMap(W, tilesetIdx, height, seed) {
  const H = W, rnd = rng(seed || 0x9A1215);
  const tiles = new Uint16Array(W * H);
  for (let i = 0; i < W * H; i++) { const r = rnd(), f = rnd() % 100; const fl = f < 60 ? 0 : f < 74 ? 1 : f < 88 ? 2 : 3; tiles[i] = (r & 3) | (fl << 14); }
  const heights = new Int16Array((W + 1) * (H + 1)).fill(height);
  return { order: [0, 1, 2, 3, 5], raw: {}, tileset: TILESETS[tilesetIdx], tilesetChanged: true, W, H, tiles, heights, props: [], attr: new Uint8Array(W * H), aw: W, ah: H,
    open: { w: 64, h: 64, data: new Uint8Array(4096).fill(255) }, fuel: { w: 64, h: 64, data: new Uint8Array(4096) } };
}
function rng(s) { let x = s >>> 0; return () => { x = (Math.imul(x, 1103515245) + 12345) >>> 0; return x >>> 16; }; }

/* ground height at a world point (two triangles per cell, rows north-first) */
function groundHeight(m, x, y) {
  const W = m.W, H = m.H;
  let fx = x / CELL, fr = H - y / CELL;
  fx = Math.max(0, Math.min(W - 1e-3, fx)); fr = Math.max(0, Math.min(H - 1e-3, fr));
  const c = Math.floor(fx), r = Math.floor(fr), u = fx - c, v = fr - r, s = W + 1;
  const h00 = m.heights[r * s + c], h10 = m.heights[r * s + c + 1], h01 = m.heights[(r + 1) * s + c], h11 = m.heights[(r + 1) * s + c + 1];
  return u + v <= 1 ? h00 + (h10 - h00) * u + (h01 - h00) * v : h11 + (h01 - h11) * (1 - u) + (h10 - h11) * (1 - v);
}

/* chunks 3 and 5 from the forest bit, after forest edits */
function rebuildForestGrids(m) {
  const W = m.W, H = m.H;
  for (const g of [m.open, m.fuel]) if (!g) return;
  const n = 64, share = new Float32Array(n * n), cnt = new Float32Array(n * n);
  for (let r = 0; r < m.ah; r++) for (let c = 0; c < m.aw; c++) {
    const k = Math.min(n - 1, Math.floor(r * n / m.ah)) * n + Math.min(n - 1, Math.floor(c * n / m.aw));
    cnt[k]++; if (m.attr[r * m.aw + c] & 0x80) share[k]++;
  }
  for (let k = 0; k < n * n; k++) {
    const f = cnt[k] ? share[k] / cnt[k] : 0;
    if (m.open.w === n && m.open.h === n) m.open.data[k] = Math.round(255 - 204 * f);
    if (m.fuel.w === n && m.fuel.h === n) { if (f >= 0.5) m.fuel.data[k] = 255; else if (m.fuel.data[k] === 255) m.fuel.data[k] = 0; }
  }
}

/* --- .stg: full model (header, troops, areas, variables, triggers, briefing troops), rebuilt byte for byte --- */
const TROOP_SZ = 0x224, RECT_SZ = 0x54;
function decName(b) { let e = b.indexOf(0); if (e < 0) e = b.length; try { return new TextDecoder('euc-kr').decode(b.subarray(0, e)); } catch (_) { return String.fromCharCode(...b.subarray(0, e)); } }
function troopFromRaw(raw, idx) {
  const v = new DataView(raw.buffer, raw.byteOffset, TROOP_SZ);
  return { raw, idx, name: decName(raw.subarray(0, 32)), id: v.getInt32(0x20, true), player: raw[0x24], control: raw[0x25], start: raw[0x26],
    x: v.getFloat32(0x48, true), y: v.getFloat32(0x4c, true), dir: raw[0x50], job: v.getInt32(0x1c4, true), unit: v.getInt16(0x194, true),
    w: v.getInt32(0x198, true), d: v.getInt32(0x19c, true), leader: raw[0x58], level: raw[0x5b] };
}
function refreshTroop(t) { const n = troopFromRaw(t.raw, t.idx); for (const k of ['name', 'id', 'player', 'control', 'start', 'job', 'unit', 'w', 'd', 'leader', 'level']) t[k] = n[k]; }
function parseStg(buf) {
  const u8 = new Uint8Array(buf).slice(), dv = new DataView(u8.buffer);
  const need = n => { if (p + n > u8.length) throw new Error('Stage file ends too early (at byte ' + p + ').'); };
  if (u8.length < 0x250 || dv.getInt32(0, true) !== 0x3e9) throw new Error('This is not a Kingdom Under Fire stage file (version 1001 expected).');
  let p = 0x248;
  const head = u8.slice(0, 0x248);
  need(4); const T = dv.getUint32(p, true); p += 4; need(T * TROOP_SZ);
  const troops = [];
  for (let i = 0; i < T; i++, p += TROOP_SZ) troops.push(troopFromRaw(u8.slice(p, p + TROOP_SZ), i));
  need(4); const R = dv.getUint32(p, true); p += 4; need(R * RECT_SZ);
  const rects = [];
  for (let i = 0; i < R; i++, p += RECT_SZ) {
    const raw = u8.slice(p, p + RECT_SZ), v = new DataView(raw.buffer);
    rects.push({ raw, name: decName(raw.subarray(0, 64)), id: v.getInt32(0x40, true), l: v.getFloat32(0x44, true), t: v.getFloat32(0x48, true), r: v.getFloat32(0x4c, true), b: v.getFloat32(0x50, true) });
  }
  const value = () => {
    need(4); const t = dv.getInt32(p, true); p += 4;
    if (t === 2) { need(4); const n = dv.getUint32(p, true); need(4 + n); const s = u8.slice(p + 4, p + 4 + n); p += 4 + n; return { t, s }; }
    if (t === 0 || t === 1 || t === 3) { need(4); const b = u8.slice(p, p + 4); p += 4; return { t, b }; }
    return { t };
  };
  const vStart = p; need(4); const V = dv.getUint32(p, true); p += 4;
  for (let i = 0; i < V; i++) { need(68); p += 68; value(); }
  const vars = u8.slice(vStart, p);
  need(4); const G = dv.getUint32(p, true); p += 4;
  const groups = [];
  const call = () => { need(8); const code = dv.getInt32(p, true), n = dv.getUint32(p + 4, true); p += 8; const params = []; for (let k = 0; k < n; k++) params.push(value()); return { code, params }; };
  for (let g = 0; g < G; g++) {
    need(8); const gid = dv.getInt32(p, true), TC = dv.getUint32(p + 4, true); p += 8;
    const triggers = [];
    for (let t = 0; t < TC; t++) {
      need(72); const name = u8.slice(p, p + 64), id = dv.getInt32(p + 64, true); p += 68;
      const nc = dv.getUint32(p, true); p += 4; const conds = []; for (let k = 0; k < nc; k++) conds.push(call());
      need(4); const na = dv.getUint32(p, true); p += 4; const acts = []; for (let k = 0; k < na; k++) acts.push(call());
      triggers.push({ name, id, conds, acts });
    }
    groups.push({ id: gid, triggers });
  }
  const tail = u8.slice(p);
  const str = o => { let s = ''; for (let i = 0; i < 64 && u8[o + i]; i++) s += String.fromCharCode(u8[o + i]); return s; };
  return { head, troops, rects, vars, groups, tail, mapName: str(0x20), miniName: str(0x60), playerType: Array.from(u8.subarray(8, 16)) };
}
const pInt = v => new DataView(v.b.buffer, v.b.byteOffset, 4).getInt32(0, true);
const mkInt = n => { const b = new Uint8Array(4); new DataView(b.buffer).setInt32(0, n, true); return { t: 0, b }; };
function setPInt(v, n) { if (!v.b) { v.t = 0; v.b = new Uint8Array(4); } new DataView(v.b.buffer, v.b.byteOffset, 4).setInt32(0, n, true); }
function buildStg(s, mapName, miniName) {
  const parts = [], u32 = n => { const b = new Uint8Array(4); new DataView(b.buffer).setUint32(0, n >>> 0, true); return b; }, i32 = n => { const b = new Uint8Array(4); new DataView(b.buffer).setInt32(0, n, true); return b; };
  const head = s.head.slice();
  const put = (o, str) => { for (let i = 0; i < 64; i++) head[o + i] = i < str.length ? str.charCodeAt(i) : 0; };
  if (mapName && mapName !== s.mapName) put(0x20, mapName);
  if (miniName && miniName !== s.miniName) put(0x60, miniName);
  parts.push(head, u32(s.troops.length));
  for (const t of s.troops) {
    const r = t.raw.slice(), v = new DataView(r.buffer);
    v.setFloat32(0x48, t.x, true); v.setFloat32(0x4c, t.y, true); r[0x50] = t.dir & 7;
    parts.push(r);
  }
  parts.push(u32(s.rects.length));
  for (const a of s.rects) {
    const r = a.raw.slice(), v = new DataView(r.buffer);
    const l = Math.min(a.l, a.r), rr = Math.max(a.l, a.r), t = Math.min(a.t, a.b), b = Math.max(a.t, a.b);
    v.setFloat32(0x44, l, true); v.setFloat32(0x48, t, true); v.setFloat32(0x4c, rr, true); v.setFloat32(0x50, b, true);
    parts.push(r);
  }
  parts.push(s.vars, u32(s.groups.length));
  const val = x => { parts.push(i32(x.t)); if (x.t === 2) { parts.push(u32(x.s.length), x.s); } else if (x.b) parts.push(x.b); };
  const callOut = c => { parts.push(i32(c.code), u32(c.params.length)); c.params.forEach(val); };
  for (const g of s.groups) {
    parts.push(i32(g.id), u32(g.triggers.length));
    for (const t of g.triggers) { parts.push(t.name, i32(t.id), u32(t.conds.length)); t.conds.forEach(callOut); parts.push(u32(t.acts.length)); t.acts.forEach(callOut); }
  }
  parts.push(s.tail);
  const out = new Uint8Array(parts.reduce((n, x) => n + x.length, 0)); let o = 0;
  for (const x of parts) { out.set(x, o); o += x.length; }
  return out;
}
/* Invasion waves: actions 183 (wave count), 184 (spawn areas + max troops), 185 (wave lines) */
function waveModel(s) {
  const lines = [], w = { begin: null, param: null, lines };
  for (const g of s.groups) for (const t of g.triggers) for (const c of t.acts) {
    if (c.code === 183 && !w.begin) w.begin = c;
    else if (c.code === 184 && !w.param) w.param = c;
    else if (c.code === 185 && c.params.length === 14) lines.push({ call: c, trig: t });
  }
  return w;
}
const lineGet = l => ({ troops: l.call.params.slice(0, 10).map(pInt).filter(x => x !== -1), ai: pInt(l.call.params[10]), from: pInt(l.call.params[11]), to: pInt(l.call.params[12]), level: pInt(l.call.params[13]) });
function lineSet(l, o) {
  const P = l.call.params;
  if (o.troops) for (let i = 0; i < 10; i++) setPInt(P[i], i < o.troops.length ? o.troops[i] : -1);
  if (o.ai != null) setPInt(P[10], o.ai); if (o.from != null) setPInt(P[11], o.from); if (o.to != null) setPInt(P[12], o.to); if (o.level != null) setPInt(P[13], o.level);
}
function addWaveLine(s, w, o) {
  const last = w.lines[w.lines.length - 1];
  const call = { code: 185, params: Array.from({ length: 14 }, () => mkInt(-1)) };
  const l = { call, trig: last ? last.trig : null };
  if (!l.trig) { const t = s.groups[0] && s.groups[0].triggers[0]; if (!t) throw new Error('This stage file has no trigger to hold wave lines.'); l.trig = t; }
  const acts = l.trig.acts, at = last ? acts.indexOf(last.call) + 1 : acts.length;
  acts.splice(at, 0, call);
  lineSet(l, { troops: [], ai: 1, from: 1, to: 1, level: 1, ...o });
  w.lines.push(l);
  return l;
}
function removeWaveLine(w, l) { const a = l.trig.acts, i = a.indexOf(l.call); if (i >= 0) a.splice(i, 1); w.lines.splice(w.lines.indexOf(l), 1); }
function addTroop(s, from, name) {
  const raw = from.raw.slice(), v = new DataView(raw.buffer);
  const id = s.troops.reduce((m, t) => Math.max(m, t.id), 0) + 1;
  v.setInt32(0x20, id, true);
  if (name != null) { const b = new Uint8Array(32); const enc = new TextEncoder().encode(name).subarray(0, 31); b.set(enc); raw.set(b, 0); }
  const t = troopFromRaw(raw, s.troops.length); s.troops.push(t); return t;
}
function copyTroopKind(dst, src) {   /* everything but name, id, side, start flags, position and facing */
  const keep = dst.raw.slice(), r = src.raw.slice();
  r.set(keep.subarray(0, 0x28), 0); r.set(keep.subarray(0x48, 0x51), 0x48);
  dst.raw = r; refreshTroop(dst);
}
/* --- DDS DXT1 (minimap) --- */
function c565(c) { return [((c >> 11) & 31) * 255 / 31 | 0, ((c >> 5) & 63) * 255 / 63 | 0, (c & 31) * 255 / 31 | 0]; }
function decodeDDS(buf) {
  const u8 = new Uint8Array(buf), dv = new DataView(u8.buffer, u8.byteOffset, u8.byteLength);
  if (u8.length < 128 || dv.getUint32(0, true) !== 0x20534444) throw new Error('Not a DDS picture.');
  const h = dv.getUint32(12, true), w = dv.getUint32(16, true), four = dv.getUint32(84, true);
  if (four !== 0x31545844) throw new Error('Only DXT1 minimaps are supported.');
  const out = new Uint8ClampedArray(w * h * 4); let o = 128;
  for (let by = 0; by < h / 4; by++) for (let bx = 0; bx < w / 4; bx++, o += 8) {
    const a = dv.getUint16(o, true), b = dv.getUint16(o + 2, true), bits = dv.getUint32(o + 4, true);
    const ca = c565(a), cb = c565(b), pal = [ca, cb];
    if (a > b) { pal.push(ca.map((v, i) => (2 * v + cb[i]) / 3 | 0), ca.map((v, i) => (v + 2 * cb[i]) / 3 | 0)); }
    else { pal.push(ca.map((v, i) => (v + cb[i]) / 2 | 0), [0, 0, 0]); }
    for (let k = 0; k < 16; k++) {
      const c = pal[(bits >>> (2 * k)) & 3], px = bx * 4 + (k & 3), py = by * 4 + (k >> 2), q = (py * w + px) * 4;
      out[q] = c[0]; out[q + 1] = c[1]; out[q + 2] = c[2]; out[q + 3] = 255;
    }
  }
  return { w, h, rgba: out };
}
function to565(r, g, b) { return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3); }
function encodeDDS(rgba, w, h) {
  const out = new Uint8Array(128 + (w / 4) * (h / 4) * 8), dv = new DataView(out.buffer);
  const H32 = [0x20534444, 124, 0x81007, h, w, w * h / 2];
  H32.forEach((v, i) => dv.setUint32(i * 4, v, true));
  dv.setUint32(76, 32, true); dv.setUint32(80, 4, true); dv.setUint32(84, 0x31545844, true); dv.setUint32(108, 0x1000, true);
  let o = 128;
  for (let by = 0; by < h / 4; by++) for (let bx = 0; bx < w / 4; bx++, o += 8) {
    let lo = 1e9, hi = -1, cl = null, ch = null; const px = [];
    for (let k = 0; k < 16; k++) {
      const q = ((by * 4 + (k >> 2)) * w + bx * 4 + (k & 3)) * 4, c = [rgba[q], rgba[q + 1], rgba[q + 2]], l = c[0] * 3 + c[1] * 6 + c[2];
      px.push(c); if (l < lo) { lo = l; cl = c; } if (l > hi) { hi = l; ch = c; }
    }
    let a = to565(...ch), b = to565(...cl);
    if (a === b) { dv.setUint16(o, a, true); dv.setUint16(o + 2, b, true); dv.setUint32(o + 4, 0, true); continue; }
    if (a < b) { const t = a; a = b; b = t; }
    const ca = c565(a), cb = c565(b), pal = [ca, cb, ca.map((v, i) => (2 * v + cb[i]) / 3), ca.map((v, i) => (v + 2 * cb[i]) / 3)];
    let bits = 0;
    for (let k = 0; k < 16; k++) {
      let best = 0, bd = 1e9;
      for (let j = 0; j < 4; j++) { const d = (px[k][0] - pal[j][0]) ** 2 + (px[k][1] - pal[j][1]) ** 2 + (px[k][2] - pal[j][2]) ** 2; if (d < bd) { bd = d; best = j; } }
      bits |= best << (2 * k);
    }
    dv.setUint16(o, a, true); dv.setUint16(o + 2, b, true); dv.setUint32(o + 4, bits >>> 0, true);
  }
  return out;
}

/* --- zip (stored) --- */
const CRC = (() => { const t = new Uint32Array(256); for (let n = 0; n < 256; n++) { let c = n; for (let k = 0; k < 8; k++) c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1; t[n] = c >>> 0; } return t; })();
function crc32(b) { let c = 0xffffffff; for (let i = 0; i < b.length; i++) c = CRC[(c ^ b[i]) & 255] ^ (c >>> 8); return (c ^ 0xffffffff) >>> 0; }
function makeZip(files) {
  const enc = new TextEncoder(), parts = [], cent = []; let off = 0;
  for (const f of files) {
    const name = enc.encode(f.name), crc = crc32(f.data), lh = new Uint8Array(30 + name.length), v = new DataView(lh.buffer);
    v.setUint32(0, 0x04034b50, true); v.setUint16(4, 20, true); v.setUint16(8, 0, true); v.setUint16(10, 0, true); v.setUint16(12, 0x21, true);
    v.setUint32(14, crc, true); v.setUint32(18, f.data.length, true); v.setUint32(22, f.data.length, true); v.setUint16(26, name.length, true); lh.set(name, 30);
    const ch = new Uint8Array(46 + name.length), cv = new DataView(ch.buffer);
    cv.setUint32(0, 0x02014b50, true); cv.setUint16(4, 20, true); cv.setUint16(6, 20, true); cv.setUint16(14, 0x21, true);
    cv.setUint32(16, crc, true); cv.setUint32(20, f.data.length, true); cv.setUint32(24, f.data.length, true); cv.setUint16(28, name.length, true); cv.setUint32(42, off, true); ch.set(name, 46);
    parts.push(lh, f.data); cent.push(ch); off += lh.length + f.data.length;
  }
  const cs = cent.reduce((s, c) => s + c.length, 0), end = new Uint8Array(22), ev = new DataView(end.buffer);
  ev.setUint32(0, 0x06054b50, true); ev.setUint16(8, files.length, true); ev.setUint16(10, files.length, true); ev.setUint32(12, cs, true); ev.setUint32(16, off, true);
  const all = new Uint8Array(off + cs + 22); let p = 0;
  for (const x of parts.concat(cent, [end])) { all.set(x, p); p += x.length; }
  return all;
}
if (typeof module !== 'undefined') module.exports = { parseMap, buildMap, newMap, parseStg, buildStg, waveModel, lineGet, lineSet, addWaveLine, removeWaveLine, addTroop, copyTroopKind, decodeDDS, encodeDDS, makeZip, groundHeight, rebuildForestGrids, tilesetIndex, crc32 };
