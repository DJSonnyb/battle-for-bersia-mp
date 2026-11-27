/* ===== Bersia Map Forge app ===== */
const $ = id => document.getElementById(id);
const BASE_COL = [[196,170,120],[96,132,62],[66,92,52],[112,92,64],[222,230,236],[120,56,60],[110,140,90],[70,104,58],[90,52,40]];
const PLAYER_COL = ['#e8c35a','#5aa9e8','#7bd389','#d98ae0','#e8735a','#9ad0d0','#c9b38a','#b0b0b0'];
const PTYPE = ['unused','human','human','enemy AI','allied AI','neutral AI'];

/* ---------- state ---------- */
const S = {
  m: null, stg: null, mini: null, tplBytes: null, tplCrc: 0, tplName: '', example: false,
  tileCol: null, tileUsed: null,
  zoom: 3, ox: 0, oy: 0,
  tool: 'select', brush: 8, strength: 40, density: 35, level: 300,
  tileSel: new Set([0, 1, 2, 3]), randFlip: true,
  propSel: null, treeSel: new Set(), facing: 0, eraseTreesOnly: true, propGroup: 'build', propFind: '',
  moveSide: false, sel: null,
  layers: { shade: true, blocked: true, obstacle: false, forest: true, slow: false, props: true, edge: true, troops: true, later: true, areas: true },
  undo: [], redo: [], dirtyForest: false,
};
let downloadsNS = null;
(async () => { try { if (window.claude && window.claude.use) downloadsNS = await window.claude.use('downloads'); } catch (_) {} })();

/* ---------- prop catalogue ---------- */
/* names from the prop table (K2PropData): walls, ruins, camps and other buildings */
const PROPNAME = { 1: 'Cliff', 7: 'Ruin piece', 8: 'Ruin piece', 9: 'Kallishire wall piece', 10: 'Kallishire wall piece', 11: 'Kallishire wall piece', 12: 'Kallishire wall piece', 13: 'Bones', 14: 'Bones',
  16: 'Tombstone', 167: 'Tombstone', 213: 'Tombstone', 401: 'Kallishire castle wall', 402: 'Kallishire wall, broken', 421: 'Ruin (small)', 422: 'Ruin (medium)', 423: 'Ruin (large)',
  425: 'Hironeiden castle wall', 426: 'Hironeiden wall, broken', 427: 'Hironeiden wall rubble', 430: 'Esse castle wall', 431: 'Esse wall, broken', 432: 'Esse wall rubble', 434: 'Ecclesia castle wall',
  458: 'Camp piece', 459: 'Camp piece', 460: 'Camp piece', 461: 'Camp piece', 462: 'Camp piece', 463: 'Camp piece', 464: 'Camp piece', 465: 'Camp piece', 466: 'Camp piece', 467: 'Camp piece', 468: 'Camp piece', 469: 'Camp piece' };
/* gates and the dam need scripts, a river or the campaign's path data: not offered */
const PROP_HIDDEN = new Set([403, 424, 428, 429, 433]);
function propGroup(id) {
  const k = propKind(id);
  if (k === 'Tree' || k === 'Undergrowth' || k === 'Small plant or prop') return k === 'Small plant or prop' ? 'other' : 'plants';
  if (/wall|Ruin|Camp|Tombstone/.test(k)) return 'build';
  if (k === 'Rock or cliff' || k === 'Cliff' || k === 'Bones') return 'rocks';
  return 'other';
}
function propKind(id) {
  if (id === 420) return 'Healing tombstone';
  if (PROPNAME[id]) return PROPNAME[id];
  const c = PROPCAT[id];
  if (!c) return 'Prop';
  const [n, forest, obst] = c;
  if (forest >= 0.9 && obst >= 0.9) return 'Tree';
  if (forest >= 0.6 && obst < 0.5) return 'Undergrowth';
  if (obst >= 0.8) return 'Rock or cliff';
  if (id >= 300 && id <= 314) return 'World-map piece';
  if (id >= 350 && id <= 362) return 'Corpse';
  if (id >= 391 && id <= 399) return 'Fire or smoke';
  return 'Small plant or prop';
}
const isTreeLike = id => { const k = propKind(id); return k === 'Tree' || k === 'Undergrowth'; };
const propTileset = id => PROPCAT[id] ? PROPCAT[id][3] : -1;

/* ---------- tile colours ---------- */
function computeTileColours() {
  const m = S.m, ts = Math.max(0, tilesetIndex(m.tileset)), base = BASE_COL[ts] || [128,128,128];
  const sum = new Float64Array(84 * 3), cnt = new Uint32Array(84), used = new Uint32Array(84);
  const src = S.origTiles && S.origTiles.length === m.W * m.H ? S.origTiles : m.tiles;
  for (let i = 0; i < m.W * m.H; i++) used[Math.min(83, m.tiles[i] & 0x3fff)]++;
  if (S.mini) {
    const mw = S.mini.w, mh = S.mini.h, px = S.mini.rgba;
    for (let r = 0; r < m.H; r++) for (let c = 0; c < m.W; c++) {
      const t = Math.min(83, src[r * m.W + c] & 0x3fff);
      const x = Math.min(mw - 1, ((c + 0.5) * mw / m.W) | 0), y = Math.min(mh - 1, ((r + 0.5) * mh / m.H) | 0), q = (y * mw + x) * 4;
      sum[t * 3] += px[q]; sum[t * 3 + 1] += px[q + 1]; sum[t * 3 + 2] += px[q + 2]; cnt[t]++;
    }
  }
  S.tileCol = [];
  for (let t = 0; t < 84; t++) {
    if (cnt[t] > 3) S.tileCol.push([sum[t * 3] / cnt[t], sum[t * 3 + 1] / cnt[t], sum[t * 3 + 2] / cnt[t]]);
    else if (t < 4) { const v = 0.97 + (t % 2) * 0.04; S.tileCol.push(base.map(x => Math.min(255, x * v))); }
    else { const earth = [150, 128, 96], k = 0.35 + ((t * 37) % 7) / 25; S.tileCol.push(base.map((x, i) => Math.min(255, x * (1 - k) + earth[i] * k))); }
  }
  S.tileUsed = used;
}

/* ---------- rendering ---------- */
const view = $('view'), vctx = view.getContext('2d');
let baseCv = document.createElement('canvas'), overCv = document.createElement('canvas');
let needBase = true, needOver = true, needDraw = true;
function shadeAt(m, r, c) {
  const s = m.W + 1, h = m.heights;
  const a = h[r * s + c], b = h[r * s + c + 1], d = h[(r + 1) * s + c], e = h[(r + 1) * s + c + 1];
  const dx = ((b + e) - (a + d)) / 2 / CELL * 2.2, dn = ((a + b) - (d + e)) / 2 / CELL * 2.2;  /* dn: rise towards north */
  const nx = -dx, ny = -dn, nz = 1, L = Math.hypot(nx, ny, nz);
  const l = (nx * -0.55 + ny * 0.55 + nz * 0.63) / L;
  return Math.max(0.35, Math.min(1.35, 0.35 + l * 0.95));
}
function renderBase() {
  const m = S.m; if (!m) return;
  if (baseCv.width !== m.W || baseCv.height !== m.H) { baseCv.width = m.W; baseCv.height = m.H; }
  const ctx = baseCv.getContext('2d'), img = ctx.createImageData(m.W, m.H), d = img.data;
  let hmin = 1e9, hmax = -1e9; for (const v of m.heights) { if (v < hmin) hmin = v; if (v > hmax) hmax = v; }
  const span = Math.max(1, hmax - hmin);
  for (let r = 0; r < m.H; r++) for (let c = 0; c < m.W; c++) {
    const i = r * m.W + c, t = m.tiles[i], col = S.tileCol[Math.min(83, t & 0x3fff)];
    let k = 1;
    if (S.layers.shade) { k = shadeAt(m, r, c); const hv = (m.heights[r * (m.W + 1) + c] - hmin) / span; k *= 0.88 + hv * 0.24; }
    const fl = (t >> 14) & 3, jit = fl === 1 ? 0.97 : fl === 2 ? 1.02 : fl === 3 ? 0.99 : 1;
    d[i * 4] = col[0] * k * jit; d[i * 4 + 1] = col[1] * k * jit; d[i * 4 + 2] = col[2] * k * jit; d[i * 4 + 3] = 255;
  }
  ctx.putImageData(img, 0, 0);
}
function renderOver() {
  const m = S.m; if (!m) return;
  if (overCv.width !== m.W || overCv.height !== m.H) { overCv.width = m.W; overCv.height = m.H; }
  const ctx = overCv.getContext('2d'), img = ctx.createImageData(m.W, m.H), d = img.data, L = S.layers;
  for (let r = 0; r < m.H; r++) for (let c = 0; c < m.W; c++) {
    const ar = Math.min(m.ah - 1, Math.floor((m.H - 1 - r) * m.ah / m.H)), ac = Math.min(m.aw - 1, Math.floor(c * m.aw / m.W));
    const a = m.attr[ar * m.aw + ac], q = (r * m.W + c) * 4;
    let R = 0, G = 0, B = 0, A = 0;
    if (L.forest && (a & 0x80)) { R = 20; G = 120; B = 40; A = 70; }
    if (L.slow && (a & 7)) { R = 150; G = 100; B = 30; A = 40 + (a & 7) * 18; }
    if (L.obstacle && (a & 0x38)) { R = 240; G = 150; B = 40; A = 150; }
    if (L.blocked && (a & 0x40)) { R = 205; G = 40; B = 35; A = ((r + c) & 1) ? 190 : 120; }
    d[q] = R; d[q + 1] = G; d[q + 2] = B; d[q + 3] = A;
  }
  ctx.putImageData(img, 0, 0);
}
function fitView() {
  const m = S.m; if (!m) return;
  const w = view.clientWidth, h = view.clientHeight;
  S.zoom = Math.max(0.5, Math.min(w / m.W, h / m.H) * 0.94);
  S.ox = (w - m.W * S.zoom) / 2; S.oy = (h - m.H * S.zoom) / 2; needDraw = true;
}
const toScreen = (cx, cy) => [S.ox + cx * S.zoom, S.oy + cy * S.zoom];
const worldToCell = (x, y) => [x / CELL, S.m.H - y / CELL];
const cellToWorld = (cx, cy) => [cx * CELL, (S.m.H - cy) * CELL];
let mouse = { sx: -1, sy: -1, inside: false };
function draw() {
  needDraw = false;
  const dpr = window.devicePixelRatio || 1, w = view.clientWidth, h = view.clientHeight;
  if (view.width !== Math.round(w * dpr) || view.height !== Math.round(h * dpr)) { view.width = Math.round(w * dpr); view.height = Math.round(h * dpr); }
  vctx.setTransform(dpr, 0, 0, dpr, 0, 0);
  vctx.fillStyle = '#0b0908'; vctx.fillRect(0, 0, w, h);
  const m = S.m; if (!m) return;
  if (needBase) { renderBase(); needBase = false; }
  if (needOver) { renderOver(); needOver = false; }
  vctx.imageSmoothingEnabled = S.zoom < 1;
  vctx.drawImage(baseCv, S.ox, S.oy, m.W * S.zoom, m.H * S.zoom);
  vctx.drawImage(overCv, S.ox, S.oy, m.W * S.zoom, m.H * S.zoom);
  const Z = S.zoom, L = S.layers;
  if (L.edge) { /* the game blocks 3000 units (24 cells) at every edge */
    const e = 3000 / CELL * Z, x0 = S.ox, y0 = S.oy, W = m.W * Z, H = m.H * Z;
    vctx.fillStyle = 'rgba(0,0,0,.38)';
    vctx.fillRect(x0, y0, W, e); vctx.fillRect(x0, y0 + H - e, W, e); vctx.fillRect(x0, y0 + e, e, H - 2 * e); vctx.fillRect(x0 + W - e, y0 + e, e, H - 2 * e);
    vctx.strokeStyle = 'rgba(215,163,94,.35)'; vctx.setLineDash([4, 4]); vctx.strokeRect(x0 + e, y0 + e, W - 2 * e, H - 2 * e); vctx.setLineDash([]);
  }
  if (L.props && Z >= 1.2) {
    const r = Math.max(1, Z * 0.32);
    for (const p of m.props) {
      const [cx, cy] = worldToCell(p.x, p.y), [sx, sy] = toScreen(cx, cy);
      if (sx < -4 || sy < -4 || sx > w + 4 || sy > h + 4) continue;
      if (p.id === 420) { vctx.fillStyle = '#7ef0c0'; vctx.fillRect(sx - 4, sy - 1.5, 8, 3); vctx.fillRect(sx - 1.5, sy - 4, 3, 8); continue; }
      const k = propKind(p.id);
      const grp = propGroup(p.id);
      vctx.fillStyle = k === 'Tree' ? 'rgba(10,52,22,.95)' : k === 'Undergrowth' ? 'rgba(40,92,40,.85)' : grp === 'build' ? 'rgba(240,120,60,.95)' : grp === 'rocks' ? 'rgba(200,190,175,.95)' : 'rgba(230,220,160,.85)';
      if (k === 'Tree' || k === 'Undergrowth') { vctx.beginPath(); vctx.arc(sx, sy, r, 0, 6.283); vctx.fill(); } else vctx.fillRect(sx - r, sy - r, 2 * r, 2 * r);
    }
  }
  if (S.stg && L.areas) {
    vctx.font = '11px "IBM Plex Mono", monospace';
    for (const a of S.stg.rects) {
      const [x1, y1] = toScreen(...worldToCell(Math.min(a.l, a.r), Math.max(a.t, a.b))), [x2, y2] = toScreen(...worldToCell(Math.max(a.l, a.r), Math.min(a.t, a.b)));
      const sel = S.sel && S.sel.type === 'rect' && S.sel.obj === a;
      vctx.strokeStyle = sel ? '#d7a35e' : 'rgba(106,166,200,.9)'; vctx.lineWidth = sel ? 2 : 1.2;
      vctx.fillStyle = sel ? 'rgba(215,163,94,.12)' : 'rgba(106,166,200,.08)';
      vctx.fillRect(x1, y1, x2 - x1, y2 - y1); vctx.strokeRect(x1, y1, x2 - x1, y2 - y1);
      vctx.fillStyle = sel ? '#d7a35e' : '#9cc8e0'; vctx.fillText('A' + a.id, x1 + 3, y1 + 12);
      if (sel) { vctx.fillStyle = '#d7a35e'; vctx.fillRect(x2 - 5, y2 - 5, 10, 10); }
    }
  }
  if (S.stg && L.troops) {
    for (const t of S.stg.troops) {
      if (!t.start && !L.later) continue;
      const [sx, sy] = toScreen(...worldToCell(t.x, t.y)), pt = S.stg.playerType[t.player] || 0;
      const col = PLAYER_COL[t.player] || '#fff', sel = S.sel && S.sel.type === 'troop' && S.sel.obj === t;
      const rad = pt === 1 || pt === 2 ? 6 : 4.5;
      vctx.lineWidth = sel ? 2.5 : 1.5;
      vctx.strokeStyle = sel ? '#fff' : '#000';
      vctx.fillStyle = col;
      vctx.beginPath();
      if (pt === 3) { vctx.moveTo(sx, sy - rad - 1); vctx.lineTo(sx + rad, sy + rad); vctx.lineTo(sx - rad, sy + rad); vctx.closePath(); }
      else vctx.arc(sx, sy, rad, 0, 6.283);
      if (t.start) vctx.fill(); else { vctx.save(); vctx.globalAlpha = 0.35; vctx.fill(); vctx.restore(); }
      vctx.stroke();
      const ang = t.dir * Math.PI / 4; /* 0 = east, 2 = north */
      vctx.strokeStyle = col; vctx.lineWidth = 1.5; vctx.beginPath(); vctx.moveTo(sx, sy); vctx.lineTo(sx + Math.cos(ang) * (rad + 6), sy - Math.sin(ang) * (rad + 6)); vctx.stroke();
      if (t.control || (pt === 1 && t.start && t.unit >= 0 && Z > 1.5 && /^P\d/.test(t.name))) { vctx.fillStyle = '#fff'; vctx.font = '600 11px "IBM Plex Sans", sans-serif'; vctx.fillText(t.name.slice(0, 10), sx + rad + 3, sy - rad); }
    }
  }
  if (mouse.inside && brushTools.has(S.tool)) {
    vctx.strokeStyle = 'rgba(255,255,255,.75)'; vctx.lineWidth = 1;
    vctx.beginPath(); vctx.arc(mouse.sx, mouse.sy, Math.max(2, S.brush * Z), 0, 6.283); vctx.stroke();
  }
}
function frame() { if (needDraw || needBase || needOver) draw(); requestAnimationFrame(frame); }
const redrawAll = () => { needBase = needOver = needDraw = true; };

/* ---------- tools ---------- */
const ICON = {
  select: '<path d="M5 3l12 8-6 1.5L8.5 19z"/>', pan: '<path d="M12 3v18M3 12h18M12 3l-3 3M12 3l3 3M12 21l-3-3M12 21l3-3M3 12l3-3M3 12l3 3M21 12l-3-3M21 12l-3 3"/>',
  raise: '<path d="M3 19c3-1 4-9 9-12 5 3 6 11 9 12"/><path d="M12 3v5M10 5l2-2 2 2"/>', lower: '<path d="M3 7c3 1 4 9 9 12 5-3 6-11 9-12"/><path d="M12 3v5M10 6l2 2 2-2"/>',
  flatten: '<path d="M3 16h18M6 12h12M3 16c2-4 4-6 6-4M21 16c-2-4-4-6-6-4"/>', smooth: '<path d="M3 15c3-5 6 3 9-2s6 3 9-1"/><path d="M3 19h18"/>',
  level: '<path d="M4 12h16M4 12l3-3M4 12l3 3M20 12l-3-3M20 12l-3 3M4 5v14M20 5v14"/>',
  paint: '<rect x="4" y="4" width="7" height="7"/><rect x="13" y="4" width="7" height="7"/><rect x="4" y="13" width="7" height="7"/><path d="M14 15l5 5M14 20l5-5"/>',
  pick: '<path d="M14 4l6 6-9 9H5v-6z"/><path d="M12 6l6 6"/>',
  block: '<rect x="4" y="4" width="16" height="16"/><path d="M4 4l16 16M20 4L4 20"/>', unblock: '<rect x="4" y="4" width="16" height="16" stroke-dasharray="3 3"/>',
  forest: '<path d="M12 3l5 8h-3l4 6H6l4-6H7z"/><path d="M12 17v4"/>', clearforest: '<path d="M12 3l5 8h-3l4 6H6l4-6H7z" stroke-dasharray="2 2"/><path d="M4 20L20 4"/>',
  prop: '<circle cx="12" cy="10" r="5"/><path d="M12 15v6M8 21h8"/>', eraseprop: '<path d="M7 17l-3-3 9-9 7 7-6 6H8z"/><path d="M11 21h9"/>',
};
const SHORT = { select: 'Select', pan: 'Move view', raise: 'Raise', lower: 'Lower', flatten: 'Flatten', smooth: 'Smooth', level: 'Set height', paint: 'Paint', pick: 'Pick', block: 'Block', unblock: 'Unblock', forest: 'Trees', clearforest: 'Clear trees', prop: 'Prop', eraseprop: 'Erase prop' };
const GROUPS = ['Troops', 'Height', 'Ground', 'Marks & trees'];
const TOOLS = [
  ['select', 'Select: troops and areas', 'Drag a troop or a spawn area to move it. Drag an area\'s corner square to resize it. With a troop selected, press Q or E to turn it.'],
  ['pan', 'Move the view', 'Drag to move around. The mouse wheel zooms anywhere; right-drag also moves the view with any tool.'],
  '-',
  ['raise', 'Raise ground', 'Paint to build hills. Heights are in centimetres; the shipped maps stay between 0 and 6375.'],
  ['lower', 'Lower ground', 'Paint to dig valleys.'],
  ['flatten', 'Flatten', 'Flattens towards the height where the stroke starts.'],
  ['smooth', 'Smooth', 'Softens bumps and sharp edges.'],
  ['level', 'Set height', 'Sets the ground to an exact height.'],
  '-',
  ['paint', 'Paint ground', 'Paints the chosen ground tiles. Tiles are the game\'s own pieces of the ground picture for this ground type.'],
  ['pick', 'Pick ground', 'Click or drag over ground you like: the tiles under the brush become the paint selection.'],
  '-',
  ['block', 'Block', 'Marks cliffs, water edges and walls (red hatching), the same way the game\'s own maps do. The game itself only uses this mark for computer troops and some abilities: troops still walk through it until the editor can make navigation files (planned).'],
  ['unblock', 'Open ground', 'Removes blocking and obstacle marks.'],
  ['forest', 'Plant forest', 'Plants trees and marks forest cover (troops hide, arrows hit less, fire spreads).'],
  ['clearforest', 'Clear forest', 'Removes trees and forest cover.'],
  ['prop', 'Place a prop', 'Click to place the chosen prop: rocks, ruins, the healing tombstone and more.'],
  ['eraseprop', 'Remove props', 'Paint over props to remove them.'],
];
const brushTools = new Set(['raise', 'lower', 'flatten', 'smooth', 'level', 'paint', 'pick', 'block', 'unblock', 'forest', 'clearforest', 'eraseprop']);
function buildRail() {
  const rail = $('rail'); rail.innerHTML = ''; let g = 0;
  const head = () => { const h = document.createElement('div'); h.className = 'grp'; h.textContent = GROUPS[g++] || ''; rail.append(h); };
  head();
  for (const t of TOOLS) {
    if (t === '-') { const s = document.createElement('div'); s.className = 'sep'; rail.append(s); head(); continue; }
    const b = document.createElement('button');
    b.className = 'tool'; b.type = 'button'; b.title = t[1]; b.setAttribute('aria-label', t[1]); b.dataset.tool = t[0];
    b.innerHTML = '<svg viewBox="0 0 24 24" aria-hidden="true">' + ICON[t[0]] + '</svg><span class="tl">' + SHORT[t[0]] + '</span>';
    b.onclick = () => setTool(t[0]);
    rail.append(b);
  }
}
function setTool(id) {
  S.tool = id;
  document.querySelectorAll('.tool').forEach(b => b.setAttribute('aria-pressed', b.dataset.tool === id ? 'true' : 'false'));
  const t = TOOLS.find(x => x[0] === id);
  $('toolTitle').textContent = t[1]; $('toolHelp').textContent = t[2];
  view.style.cursor = id === 'pan' ? 'grab' : id === 'select' ? 'default' : 'crosshair';
  buildToolOpts(); needDraw = true;
}
function slider(label, key, min, max, step, fmt) {
  const id = 'o_' + key, row = document.createElement('div'); row.className = 'row';
  row.innerHTML = `<label for="${id}">${label}</label><input type="range" id="${id}" min="${min}" max="${max}" step="${step}" value="${S[key]}"><span class="val" id="${id}v"></span>`;
  const inp = row.querySelector('input'), v = row.querySelector('.val');
  const show = () => v.textContent = fmt ? fmt(S[key]) : S[key];
  inp.oninput = () => { S[key] = +inp.value; show(); needDraw = true; }; show();
  return row;
}
function buildToolOpts() {
  const box = $('toolOpts'); box.innerHTML = '';
  const id = S.tool;
  if (brushTools.has(id)) box.append(slider('Brush', 'brush', 1, 40, 1, v => v + ' cells'));
  if (['raise', 'lower', 'flatten', 'smooth', 'level', 'paint', 'forest'].includes(id)) box.append(slider('Strength', 'strength', 1, 100, 1, v => v + '%'));
  if (id === 'level') {
    const r = document.createElement('div'); r.className = 'row';
    r.innerHTML = '<label for="o_lv">Height</label><input type="number" id="o_lv" min="0" max="6375" step="25" style="width:110px"><span class="note">cm</span>';
    const i = r.querySelector('input'); i.value = S.level; i.oninput = () => S.level = Math.max(0, Math.min(6375, +i.value || 0)); box.append(r);
  }
  if (id === 'paint' || id === 'pick') {
    const p = document.createElement('div'); p.className = 'tiles'; p.id = 'tilePal';
    for (let t = 0; t < 84; t++) {
      const b = document.createElement('button'); b.type = 'button';
      const c = S.tileCol ? S.tileCol[t] : [90, 90, 90];
      b.style.background = `rgb(${c.map(v => v | 0).join(',')})`;
      const n = S.tileUsed ? S.tileUsed[t] : 0;
      b.title = 'Tile ' + t + (n ? ' (' + n + ' cells in this map)' : ' (not used in this map)');
      b.setAttribute('aria-label', b.title);
      if (!n) b.classList.add('unused');
      b.setAttribute('aria-pressed', S.tileSel.has(t) ? 'true' : 'false');
      b.onclick = () => { if (S.tileSel.has(t)) { if (S.tileSel.size > 1) S.tileSel.delete(t); } else S.tileSel.add(t); buildToolOpts(); };
      p.append(b);
    }
    box.append(p);
    const n = document.createElement('p'); n.className = 'note';
    n.innerHTML = `Selected: <b>${[...S.tileSel].sort((a, b) => a - b).join(', ')}</b>. Tiles 0–3 are the plain ground of every ground type; the others are its paths, patches and edges. The colours come from the minimap, so open the .dds as well for a truer picture.`;
    box.append(n);
    const r = document.createElement('div'); r.className = 'layers';
    r.innerHTML = '<label><input type="checkbox" id="o_flip"> Random turns</label>';
    const cb = r.querySelector('input'); cb.checked = S.randFlip; cb.onchange = () => S.randFlip = cb.checked; box.append(r);
  }
  if (id === 'forest') {
    box.append(slider('Trees', 'density', 5, 100, 5, v => v + '%'));
    box.append(propPicker(true));
  }
  if (id === 'prop') {
    const r = document.createElement('div'); r.className = 'row';
    r.innerHTML = '<label for="o_face">Facing</label><input type="range" id="o_face" min="0" max="345" step="15"><span class="val"></span>';
    const i = r.querySelector('input'), v = r.querySelector('.val'); i.value = S.facing; v.textContent = S.facing + '°';
    i.oninput = () => { S.facing = +i.value; v.textContent = S.facing + '°'; }; box.append(r);
    box.append(propPicker(false));
  }
  if (id === 'eraseprop' || id === 'clearforest') {
    const r = document.createElement('div'); r.className = 'layers';
    r.innerHTML = '<label><input type="checkbox" id="o_tro"> Trees and undergrowth only</label>';
    const cb = r.querySelector('input'); cb.checked = S.eraseTreesOnly; cb.onchange = () => S.eraseTreesOnly = cb.checked;
    if (id === 'eraseprop') box.append(r);
  }
  if (id === 'select') {
    const r = document.createElement('div'); r.className = 'layers';
    r.innerHTML = '<label><input type="checkbox" id="o_side"> Move a troop\'s whole side together</label>';
    const cb = r.querySelector('input'); cb.checked = S.moveSide; cb.onchange = () => S.moveSide = cb.checked; box.append(r);
    if (S.sel) box.append(selInfo());
    if (S.stg) {
      const r2 = document.createElement('div'); r2.className = 'row';
      const add = document.createElement('button'); add.type = 'button'; add.className = 'btn'; add.textContent = 'Add an area'; add.onclick = addArea;
      r2.append(add);
      if (S.sel && S.sel.type === 'rect') { const del = document.createElement('button'); del.type = 'button'; del.className = 'btn'; del.textContent = 'Delete this area'; del.onclick = deleteArea; r2.append(del); }
      box.append(r2);
    }
  }
}
function addArea() {
  if (!S.stg || !S.m) return; pushUndo();
  const s = S.stg, id = s.rects.reduce((m, a) => Math.max(m, a.id), -1) + 1;
  const raw = new Uint8Array(0x54), v = new DataView(raw.buffer);
  const nm = new TextEncoder().encode('Area' + String(id).padStart(2, '0')); raw.set(nm.subarray(0, 63), 0);
  v.setInt32(0x40, id, true);
  const cx = S.m.W * CELL / 2, cy = S.m.H * CELL / 2;
  const a = { raw, name: 'Area' + String(id).padStart(2, '0'), id, l: cx - 1500, t: cy - 1500, r: cx + 1500, b: cy + 1500 };
  s.rects.push(a);
  selectObj({ type: 'rect', obj: a }); toast('Added area A' + id + ' in the middle of the map. Drag it into place; pick it as a spawn area in Waves.');
}
function deleteArea() {
  if (!S.sel || S.sel.type !== 'rect') return; pushUndo();
  const s = S.stg, a = S.sel.obj, w = waveModel(s);
  s.rects.splice(s.rects.indexOf(a), 1);
  let used = 0;
  if (w.param) for (let k = 0; k < 6; k++) if (pInt(w.param.params[k]) === a.id) { setPInt(w.param.params[k], -1); used++; }
  S.sel = null; buildToolOpts(); buildTroopList(); needDraw = true;
  if (!$('waves').hidden) buildWaves();
  toast('Deleted area A' + a.id + (used ? ' and took it out of the wave spawn areas.' : '.'));
}
function propPicker(trees) {
  const wrap = document.createElement('div'); wrap.style.display = 'grid'; wrap.style.gap = '6px';
  const m = S.m, ts = m ? tilesetIndex(m.tileset) : -1, inMap = new Map();
  if (m) for (const p of m.props) inMap.set(p.id, (inMap.get(p.id) || 0) + 1);
  let ids = Object.keys(PROPCAT).map(Number);
  if (!ids.includes(420)) ids.push(420);
  ids = ids.filter(id => !PROP_HIDDEN.has(id) && (trees ? isTreeLike(id) : (S.propGroup === 'all' || propGroup(id) === S.propGroup)));
  if (!trees && S.propFind) { const q = S.propFind.toLowerCase(); ids = ids.filter(id => String(id) === q || propKind(id).toLowerCase().includes(q)); }
  ids.sort((a, b) => ((inMap.has(b) ? 2 : 0) + (propTileset(b) === ts ? 1 : 0)) - ((inMap.has(a) ? 2 : 0) + (propTileset(a) === ts ? 1 : 0)) || a - b);
  if (trees && S.treeSel.size === 0) for (const id of ids) if (inMap.has(id) && propKind(id) === 'Tree') S.treeSel.add(id);
  if (trees && S.treeSel.size === 0) for (const id of ids) if (propTileset(id) === ts && propKind(id) === 'Tree') { S.treeSel.add(id); if (S.treeSel.size >= 3) break; }
  if (!trees && S.propSel == null) S.propSel = 420;
  const note = document.createElement('p'); note.className = 'note';
  note.innerHTML = trees ? 'Trees to plant (pick several to mix them). Ones already in this map come first, then ones made for this ground type.' : 'Props already in this map come first. The editor shows them as dots (walls and buildings in orange); you see the real model in the game. Walls and buildings do not stop troops: they walk through them until the editor can make navigation files (planned).';
  if (!trees) {
    const bar = document.createElement('div'); bar.className = 'row';
    const sel = document.createElement('select'); sel.id = 'o_pgroup'; sel.setAttribute('aria-label', 'Prop type');
    for (const [v, t] of [['build', 'Walls, ruins and camps'], ['rocks', 'Rocks and cliffs'], ['plants', 'Trees and plants'], ['other', 'Other props'], ['all', 'Everything']]) { const o = document.createElement('option'); o.value = v; o.textContent = t; sel.append(o); }
    sel.value = S.propGroup; sel.onchange = () => { S.propGroup = sel.value; buildToolOpts(); };
    const find = document.createElement('input'); find.type = 'text'; find.id = 'o_pfind'; find.placeholder = 'Search name or number'; find.value = S.propFind || '';
    find.oninput = () => { S.propFind = find.value; const keep = find.selectionStart; buildToolOpts(); const f = $('o_pfind'); if (f) { f.focus(); f.setSelectionRange(keep, keep); } };
    bar.append(sel, find); wrap.append(bar);
  }
  const list = document.createElement('div'); list.className = 'list';
  for (const id of ids) {
    const b = document.createElement('button'); b.type = 'button'; b.className = 'item';
    const on = trees ? S.treeSel.has(id) : S.propSel === id;
    b.setAttribute('aria-pressed', on ? 'true' : 'false');
    const pts = propTileset(id), tsName = pts >= 0 ? TILESET_NAMES[pts] : '';
    b.innerHTML = `<span class="n">${id}</span><span class="t">${propKind(id)}${tsName ? ' · ' + tsName : ''}</span><span class="n">${inMap.has(id) ? inMap.get(id) + ' here' : ''}</span>`;
    b.onclick = () => { if (trees) { if (S.treeSel.has(id)) S.treeSel.delete(id); else S.treeSel.add(id); } else S.propSel = id; buildToolOpts(); };
    list.append(b);
  }
  wrap.append(note, list);
  return wrap;
}
function selInfo() {
  const s = S.sel, d = document.createElement('dl'); d.className = 'kv';
  if (s.type === 'troop') {
    const t = s.obj, pt = S.stg.playerType[t.player] || 0;
    d.innerHTML = `<dt>Troop</dt><dd>${esc(t.name) || '#' + t.idx}</dd><dt>Side</dt><dd>Player ${t.player + 1} (${PTYPE[pt] || '?'})</dd><dt>At start</dt><dd>${t.start ? 'yes' : 'no, arrives later'}</dd><dt>Position</dt><dd>${(t.x / 100).toFixed(1)} m, ${(t.y / 100).toFixed(1)} m</dd><dt>Facing</dt><dd>${['east','north-east','north','north-west','west','south-west','south','south-east'][t.dir & 7]}</dd>`;
  } else {
    const a = s.obj;
    d.innerHTML = `<dt>Area</dt><dd>A${a.id} ${esc(a.name)}</dd><dt>West–east</dt><dd>${(Math.min(a.l, a.r) / 100).toFixed(1)}–${(Math.max(a.l, a.r) / 100).toFixed(1)} m</dd><dt>South–north</dt><dd>${(Math.min(a.t, a.b) / 100).toFixed(1)}–${(Math.max(a.t, a.b) / 100).toFixed(1)} m</dd>`;
  }
  return d;
}
const esc = s => String(s).replace(/[&<>"]/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]));

/* ---------- layers panel ---------- */
const LAYERS = [['shade', 'Hill shading', '#8a7a68'], ['blocked', 'Blocked', '#cd2823'], ['forest', 'Forest cover', '#147828'], ['obstacle', 'Obstacles', '#f09628'], ['slow', 'Slow ground', '#96641e'], ['props', 'Trees and props', '#0a3416'], ['edge', 'Map edge (no entry)', '#000'], ['areas', 'Spawn areas', '#6aa6c8'], ['troops', 'Troops', '#e8c35a'], ['later', 'Troops that arrive later', '#7a6a3a']];
function buildLayers() {
  const box = $('layers'); box.innerHTML = '';
  for (const [k, label, col] of LAYERS) {
    const l = document.createElement('label');
    l.innerHTML = `<input type="checkbox" id="L_${k}"><span class="swatch" style="background:${col}"></span>${label}`;
    const cb = l.querySelector('input'); cb.checked = S.layers[k];
    cb.onchange = () => { S.layers[k] = cb.checked; if (k === 'shade') needBase = true; needOver = true; needDraw = true; };
    box.append(l);
  }
}

/* ---------- info, troops ---------- */
function refreshInfo() {
  const m = S.m, info = $('info');
  if (!m) { info.innerHTML = ''; return; }
  let hmin = 1e9, hmax = -1e9; for (const v of m.heights) { hmin = Math.min(hmin, v); hmax = Math.max(hmax, v); }
  let blocked = 0, forest = 0; for (const a of m.attr) { if (a & 0x40) blocked++; if (a & 0x80) forest++; }
  const trees = m.props.filter(p => propKind(p.id) === 'Tree').length;
  info.innerHTML = `<dt>Source</dt><dd>${esc(S.tplName || 'new map')}</dd><dt>Size</dt><dd>${m.W} × ${m.H} cells (${m.W * 1.25} m)</dd><dt>Heights</dt><dd>${hmin}–${hmax} cm</dd><dt>Trees</dt><dd>${trees}</dd><dt>Other props</dt><dd>${m.props.length - trees}</dd><dt>Blocked</dt><dd>${(100 * blocked / m.attr.length).toFixed(1)}%</dd><dt>Forest</dt><dd>${(100 * forest / m.attr.length).toFixed(1)}%</dd>` + (S.stg ? `<dt>Stage file</dt><dd>${S.stg.troops.length} troops, ${S.stg.rects.length} areas</dd>` : '');
  const sel = $('tileset'); sel.value = String(Math.max(0, tilesetIndex(m.tileset)));
}
let infoTimer = 0;
const refreshInfoSoon = () => { clearTimeout(infoTimer); infoTimer = setTimeout(refreshInfo, 250); };
function buildTroopList() {
  const list = $('troopList'); list.innerHTML = '';
  if (!S.stg) { $('troopNote').hidden = false; return; }
  $('troopNote').hidden = true;
  const s = S.stg, byP = new Map();
  if (isVersusStage(s)) {
    const n = s.playerType.filter(x => x === 1).length;
    const row = el('div', { class: 'row' }, el('label', { for: 'nPlayers', text: 'Players' }));
    const sel = el('select', { id: 'nPlayers' });
    for (let k = 2; k <= 6; k++) sel.append(el('option', { value: String(k), text: k + ' players' }));
    sel.value = String(n);
    sel.onchange = () => { pushUndo(); setPlayers(+sel.value); };
    row.append(sel);
    list.append(row, el('p', { class: 'note', text: 'Each player starts with 5 troops. New players start in the middle of the map: drag them into place (whole side moves together).' }));
  }
  for (const t of s.troops) { if (!byP.has(t.player)) byP.set(t.player, []); byP.get(t.player).push(t); }
  for (const [p, ts] of [...byP.entries()].sort((a, b) => a[0] - b[0])) {
    const now = ts.filter(t => t.start).length, pt = s.playerType[p] || 0;
    const b = document.createElement('button'); b.type = 'button'; b.className = 'item';
    b.title = 'Click to find this side on the map. "later" = troops that are not on the field at the start (waves or reinforcements).'; b.innerHTML = `<span class="dot" style="background:${PLAYER_COL[p] || '#fff'}"></span><span class="t">Player ${p + 1} · ${PTYPE[pt] || 'unknown'}</span><span class="n">${ts.length - now ? now + ' + ' + (ts.length - now) + ' later' : now + ' troops'}</span>`;
    b.onclick = () => { const t = ts.find(x => x.start) || ts[0]; selectObj({ type: 'troop', obj: t }); centerOn(t.x, t.y); };
    list.append(b);
  }
  for (const a of s.rects) {
    const b = document.createElement('button'); b.type = 'button'; b.className = 'item';
    b.setAttribute('aria-pressed', S.sel && S.sel.obj === a ? 'true' : 'false');
    b.innerHTML = `<span class="dot" style="background:var(--sea);border-radius:2px"></span><span class="t">Area ${a.id} ${esc(a.name)}</span><span class="n">${Math.round(Math.abs(a.r - a.l) / 100)}×${Math.round(Math.abs(a.b - a.t) / 100)} m</span>`;
    b.onclick = () => { selectObj({ type: 'rect', obj: a }); centerOn((a.l + a.r) / 2, (a.t + a.b) / 2); };
    list.append(b);
  }
}
function selectObj(s) { S.sel = s; if (S.tool !== 'select') setTool('select'); else buildToolOpts(); buildTroopList(); needDraw = true; }
function centerOn(x, y) { const [cx, cy] = worldToCell(x, y); S.ox = view.clientWidth / 2 - cx * S.zoom; S.oy = view.clientHeight / 2 - cy * S.zoom; needDraw = true; }

/* ---------- Versus player count ---------- */
function isVersusStage(s) { return s.playerType.every(x => x === 0 || x === 1) && !waveModel(s).lines.length && s.troops.length && s.troops.every(t => s.playerType[t.player] === 1); }
function setPlayers(n) {
  const s = S.stg, cur = s.playerType.filter(x => x === 1).length, m = S.m;
  if (n < cur) s.troops = s.troops.filter(t => t.player < n);
  for (let k = cur; k < n; k++) {
    const src = s.troops.filter(t => t.player === 0);
    const cx = m.W * CELL / 2, cy = m.H * CELL / 2, ang = k * Math.PI / 3, ox = Math.cos(ang) * 1500, oy = Math.sin(ang) * 1500;
    const sx = src.reduce((a, t) => a + t.x, 0) / src.length, sy = src.reduce((a, t) => a + t.y, 0) / src.length;
    for (const t0 of src) {
      const t = addTroop(s, t0, null);
      t.raw[0x24] = k; refreshTroop(t);
      t.x = cx + ox + (t0.x - sx); t.y = cy + oy + (t0.y - sy); t.dir = t0.dir;
    }
  }
  s.troops.forEach((t, i) => t.idx = i);
  for (let k = 0; k < 8; k++) { const on = k < n; s.head[8 + k] = on ? 1 : 0; s.head[0x10 + k] = on ? k : 0; if (!on) s.head[0x18 + k] = 0; }
  s.playerType = Array.from(s.head.subarray(8, 16));
  const nm = $('expName').value.replace(/^\d_/, n + '_'); if (/^\d_/.test($('expName').value)) { $('expName').value = nm; showNameHint(); }
  S.sel = null; buildTroopList(); refreshInfo(); needDraw = true;
  toast(n > cur ? `Added ${n - cur} player start${n - cur > 1 ? 's' : ''} in the middle of the map - drag them into place.` : `The map now has ${n} player starts.`, 6000);
}

/* ---------- undo ---------- */
function snapshot() {
  const m = S.m;
  return { tiles: m.tiles.slice(), heights: m.heights.slice(), attr: m.attr.slice(), props: m.props.map(p => ({ ...p })), tileset: m.tileset, tsc: m.tilesetChanged,
    open: m.open ? m.open.data.slice() : null, fuel: m.fuel ? m.fuel.data.slice() : null,
    stg: S.stg ? buildStg(S.stg) : null };
}
function restore(s) {
  const m = S.m;
  m.tiles = s.tiles; m.heights = s.heights; m.attr = s.attr; m.props = s.props; m.tileset = s.tileset; m.tilesetChanged = s.tsc;
  if (s.open) m.open.data = s.open; if (s.fuel) m.fuel.data = s.fuel;
  if (s.stg) { S.stg = parseStg(s.stg); S.sel = null; buildTroopList(); if (!$('waves').hidden) buildWaves(); }
  computeTileColours(); redrawAll(); refreshInfo(); buildToolOpts();
}
function pushUndo() { if (!S.m) return; S.undo.push(snapshot()); if (S.undo.length > 30) S.undo.shift(); S.redo = []; updUndo(); }
function doUndo() { if (!S.undo.length) return; S.redo.push(snapshot()); restore(S.undo.pop()); updUndo(); }
function doRedo() { if (!S.redo.length) return; S.undo.push(snapshot()); restore(S.redo.pop()); updUndo(); }
function updUndo() { $('bUndo').disabled = !S.undo.length; $('bRedo').disabled = !S.redo.length; }

/* ---------- brushes ---------- */
let stroke = null;
const rand = Math.random;
function cellsInBrush(cx, cy, rad, fn) {
  const m = S.m, r2 = rad * rad;
  for (let r = Math.max(0, Math.floor(cy - rad)); r <= Math.min(m.H - 1, Math.ceil(cy + rad)); r++)
    for (let c = Math.max(0, Math.floor(cx - rad)); c <= Math.min(m.W - 1, Math.ceil(cx + rad)); c++) {
      const d2 = (c + 0.5 - cx) ** 2 + (r + 0.5 - cy) ** 2; if (d2 <= r2) fn(r, c, Math.sqrt(d2) / rad);
    }
}
const attrIndex = (r, c) => { const m = S.m; return Math.min(m.ah - 1, Math.floor((m.H - 1 - r) * m.ah / m.H)) * m.aw + Math.min(m.aw - 1, Math.floor(c * m.aw / m.W)); };
const fall = d => { const t = 1 - d; return t * t * (3 - 2 * t); };
function dab(cx, cy) {
  const m = S.m, tool = S.tool, rad = S.brush, st = S.strength / 100;
  if (['raise', 'lower', 'flatten', 'smooth', 'level'].includes(tool)) {
    const s = m.W + 1, src = tool === 'smooth' ? m.heights.slice() : null;
    for (let r = Math.max(0, Math.floor(cy - rad)); r <= Math.min(m.H, Math.ceil(cy + rad)); r++)
      for (let c = Math.max(0, Math.floor(cx - rad)); c <= Math.min(m.W, Math.ceil(cx + rad)); c++) {
        const d = Math.hypot(c - cx, r - cy) / rad; if (d > 1) continue;
        const f = fall(d), i = r * s + c; let h = m.heights[i];
        if (tool === 'raise') h += 40 * st * f;
        else if (tool === 'lower') h -= 40 * st * f;
        else if (tool === 'flatten') h += (stroke.target - h) * Math.min(1, st * f * 0.6);
        else if (tool === 'level') h += (S.level - h) * Math.min(1, st * f * 1.2);
        else { let sum = 0, n = 0; for (let dr = -1; dr <= 1; dr++) for (let dc = -1; dc <= 1; dc++) { const rr = r + dr, cc = c + dc; if (rr >= 0 && rr <= m.H && cc >= 0 && cc <= m.W) { sum += src[rr * s + cc]; n++; } } h += (sum / n - h) * Math.min(1, st * f * 0.8); }
        m.heights[i] = Math.max(0, Math.min(6375, Math.round(h)));
      }
    stroke.heights = true; needBase = true;
  } else if (tool === 'paint') {
    const sel = [...S.tileSel];
    cellsInBrush(cx, cy, rad, (r, c) => {
      if (stroke.done.has(r * m.W + c) || rand() > st) return; stroke.done.add(r * m.W + c);
      const i = r * m.W + c, t = sel[(rand() * sel.length) | 0];
      const fl = S.randFlip ? (() => { const f = rand() * 100; return f < 60 ? 0 : f < 74 ? 1 : f < 88 ? 2 : 3; })() : (m.tiles[i] >> 14) & 3;
      m.tiles[i] = (t & 0x3fff) | (fl << 14);
    });
    needBase = true;
  } else if (tool === 'pick') {
    if (!stroke.picked) { stroke.picked = new Set(); }
    cellsInBrush(cx, cy, rad, (r, c) => stroke.picked.add(m.tiles[r * m.W + c] & 0x3fff));
    S.tileSel = new Set(stroke.picked);
  } else if (tool === 'block' || tool === 'unblock') {
    cellsInBrush(cx, cy, rad, (r, c) => { const k = attrIndex(r, c); if (tool === 'block') m.attr[k] |= 0x40; else m.attr[k] &= ~0x78; });
    needOver = true;
  } else if (tool === 'forest') {
    const ids = [...S.treeSel]; if (!ids.length) return;
    if (!stroke.occ) { stroke.occ = new Set(); for (const p of m.props) if (isTreeLike(p.id)) stroke.occ.add(Math.floor(m.H - p.y / CELL) * m.W + Math.floor(p.x / CELL)); stroke.nextInst = m.props.reduce((a, p) => Math.max(a, p.inst), 0) + 1; }
    cellsInBrush(cx, cy, rad, (r, c, d) => {
      const key = r * m.W + c; if (stroke.done.has(key)) return; stroke.done.add(key);
      if (rand() > st * fall(d * 0.85)) return;
      const k = attrIndex(r, c); m.attr[k] |= 0x80;
      if (!stroke.occ.has(key) && rand() < S.density / 100) {
        const id = ids[(rand() * ids.length) | 0], x = (c + 0.5) * CELL, y = (m.H - r - 0.5) * CELL;
        const yaw = (rand() * 16) | 0, sc = (rand() * 16) | 0;
        m.props.push({ id, inst: stroke.nextInst++, x, y, h: groundHeight(m, x, y), rot: ((yaw << 20) | (sc << 16) | (((sc << 4) | yaw) & 0xff)) >>> 0 });
        stroke.occ.add(key);
        if (propKind(id) === 'Tree') m.attr[k] |= 0x10;
      }
    });
    S.dirtyForest = true; needOver = true;
  } else if (tool === 'clearforest' || tool === 'eraseprop') {
    const r2 = (rad * CELL) ** 2, [wx, wy] = cellToWorld(cx, cy);
    const only = tool === 'clearforest' || S.eraseTreesOnly;
    m.props = m.props.filter(p => !(((p.x - wx) ** 2 + (p.y - wy) ** 2) <= r2 && (!only || isTreeLike(p.id))));
    if (tool === 'clearforest') { cellsInBrush(cx, cy, rad, (r, c) => { m.attr[attrIndex(r, c)] &= ~0x90; }); S.dirtyForest = true; needOver = true; }
  }
  needDraw = true;
}
function endStroke() {
  if (!stroke) return;
  const m = S.m;
  if (stroke.heights) for (const p of m.props) p.h = groundHeight(m, p.x, p.y);
  if (S.tool === 'pick') { buildToolOpts(); toast('Picked ' + S.tileSel.size + ' tile' + (S.tileSel.size === 1 ? '' : 's') + '. Switch to Paint ground to use them.'); }
  stroke = null; refreshInfoSoon();
}

/* ---------- select / drag troops and areas ---------- */
function hitTest(sx, sy) {
  if (!S.stg) return null;
  if (S.layers.troops) {
    let best = null, bd = 12;
    for (const t of S.stg.troops) { if (!t.start && !S.layers.later) continue; const [x, y] = toScreen(...worldToCell(t.x, t.y)); const d = Math.hypot(x - sx, y - sy); if (d < bd) { bd = d; best = t; } }
    if (best) return { type: 'troop', obj: best };
  }
  if (S.layers.areas) {
    for (const a of S.stg.rects) {
      const [x1, y1] = toScreen(...worldToCell(Math.min(a.l, a.r), Math.max(a.t, a.b))), [x2, y2] = toScreen(...worldToCell(Math.max(a.l, a.r), Math.min(a.t, a.b)));
      if (Math.abs(sx - x2) < 8 && Math.abs(sy - y2) < 8) return { type: 'rect', obj: a, corner: true };
    }
    for (const a of S.stg.rects.slice().reverse()) {
      const [x1, y1] = toScreen(...worldToCell(Math.min(a.l, a.r), Math.max(a.t, a.b))), [x2, y2] = toScreen(...worldToCell(Math.max(a.l, a.r), Math.min(a.t, a.b)));
      if (sx >= x1 && sx <= x2 && sy >= y1 && sy <= y2) return { type: 'rect', obj: a };
    }
  }
  return null;
}
const clampW = v => Math.max(0, Math.min(S.m.W * CELL, v));
const clampH = v => Math.max(0, Math.min(S.m.H * CELL, v));

/* ---------- pointer ---------- */
let drag = null;
function cellAt(e) { const r = view.getBoundingClientRect(); const sx = e.clientX - r.left, sy = e.clientY - r.top; return { sx, sy, cx: (sx - S.ox) / S.zoom, cy: (sy - S.oy) / S.zoom }; }
view.addEventListener('contextmenu', e => e.preventDefault());
view.addEventListener('pointerdown', e => {
  if (!S.m) return;
  view.setPointerCapture(e.pointerId);
  const p = cellAt(e);
  if (e.button === 1 || e.button === 2 || S.tool === 'pan') { drag = { pan: true, sx: p.sx, sy: p.sy, ox: S.ox, oy: S.oy }; view.style.cursor = 'grabbing'; return; }
  if (S.tool === 'select') {
    const h = hitTest(p.sx, p.sy);
    S.sel = h; buildToolOpts(); buildTroopList();
    if (h) { pushUndo(); const [wx, wy] = cellToWorld(p.cx, p.cy); drag = { sel: h, wx, wy }; }
    needDraw = true; return;
  }
  if (S.tool === 'prop') {
    if (S.propSel == null) return;
    pushUndo();
    const m = S.m, c = Math.floor(p.cx), r = Math.floor(p.cy);
    if (c < 0 || r < 0 || c >= m.W || r >= m.H) return;
    const [x, y] = e.shiftKey ? cellToWorld(p.cx, p.cy) : [(c + 0.5) * CELL, (m.H - r - 0.5) * CELL];
    const inst = m.props.reduce((a, q) => Math.max(a, q.inst), 0) + 1, yaw = Math.round(S.facing / 360 * 1024) & 0x3ff;
    m.props.push({ id: S.propSel, inst, x, y, h: groundHeight(m, x, y), rot: S.propSel === 420 ? (0x00640000 | yaw) >>> 0 : yaw });
    if (propGroup(S.propSel) === 'rocks' || propGroup(S.propSel) === 'build') m.attr[attrIndex(r, c)] |= 0x20;
    needOver = true; needDraw = true; refreshInfoSoon(); return;
  }
  if (brushTools.has(S.tool)) {
    pushUndo();
    stroke = { done: new Set(), target: groundHeight(S.m, ...cellToWorld(p.cx, p.cy)) };
    dab(p.cx, p.cy);
  }
});
view.addEventListener('pointermove', e => {
  const p = cellAt(e); mouse = { sx: p.sx, sy: p.sy, inside: true };
  if (S.m) {
    const [wx, wy] = cellToWorld(p.cx, p.cy);
    if (p.cx >= 0 && p.cy >= 0 && p.cx < S.m.W && p.cy < S.m.H) $('hudPos').textContent = `${(wx / 100).toFixed(1)} m E · ${(wy / 100).toFixed(1)} m N · height ${Math.round(groundHeight(S.m, wx, wy))} cm`;
    else $('hudPos').textContent = 'outside the map';
  }
  if (drag && drag.pan) { S.ox = drag.ox + p.sx - drag.sx; S.oy = drag.oy + p.sy - drag.sy; needDraw = true; return; }
  if (drag && drag.sel) {
    const [wx, wy] = cellToWorld(p.cx, p.cy), dx = wx - drag.wx, dy = wy - drag.wy; drag.wx = wx; drag.wy = wy;
    const s = drag.sel;
    if (s.type === 'troop') {
      const list = S.moveSide ? S.stg.troops.filter(t => t.player === s.obj.player) : [s.obj];
      for (const t of list) { t.x = clampW(t.x + dx); t.y = clampH(t.y + dy); }
    } else if (s.corner) { const a = s.obj; if (a.r >= a.l) a.r = clampW(a.r + dx); else a.l = clampW(a.l + dx); if (a.t <= a.b) a.t = clampH(a.t + dy); else a.b = clampH(a.b + dy); }
    else { const a = s.obj; a.l = clampW(a.l + dx); a.r = clampW(a.r + dx); a.t = clampH(a.t + dy); a.b = clampH(a.b + dy); }
    needDraw = true; return;
  }
  if (stroke) dab(p.cx, p.cy);
  needDraw = true;
});
const endPointer = () => { if (drag && drag.sel) { buildToolOpts(); buildTroopList(); } drag = null; endStroke(); view.style.cursor = S.tool === 'pan' ? 'grab' : S.tool === 'select' ? 'default' : 'crosshair'; };
view.addEventListener('pointerup', endPointer);
view.addEventListener('pointercancel', endPointer);
view.addEventListener('pointerleave', () => { mouse.inside = false; needDraw = true; });
view.addEventListener('wheel', e => {
  if (!S.m) return; e.preventDefault();
  const p = cellAt(e), f = Math.exp(-e.deltaY * 0.0015), z = Math.max(0.4, Math.min(24, S.zoom * f));
  S.ox = p.sx - p.cx * z; S.oy = p.sy - p.cy * z; S.zoom = z; $('hudZoom').textContent = Math.round(z * 100 / 3) + '%'; needDraw = true;
}, { passive: false });
window.addEventListener('keydown', e => {
  if (e.target.matches('input, select, textarea')) return;
  if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'z') { e.preventDefault(); e.shiftKey ? doRedo() : doUndo(); }
  else if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'y') { e.preventDefault(); doRedo(); }
  else if (e.key === '[') { S.brush = Math.max(1, S.brush - 1); buildToolOpts(); needDraw = true; }
  else if (e.key === ']') { S.brush = Math.min(40, S.brush + 1); buildToolOpts(); needDraw = true; }
  else if ((e.key === 'Delete' || e.key === 'Backspace') && S.sel && S.sel.type === 'rect') { e.preventDefault(); deleteArea(); }
  else if ((e.key === 'q' || e.key === 'e') && S.sel && S.sel.type === 'troop') { pushUndo(); const t = S.sel.obj; t.dir = (t.dir + (e.key === 'q' ? 1 : 7)) & 7; buildToolOpts(); needDraw = true; }
});
window.addEventListener('resize', () => { needDraw = true; });

/* ---------- files ---------- */
function toast(msg, ms) { const t = $('toast'); t.textContent = msg; t.hidden = false; clearTimeout(toast.t); toast.t = setTimeout(() => t.hidden = true, ms || 4200); }
async function openFiles(files) {
  $('drop').hidden = true;
  const arr = [...files]; if (!arr.length) return;
  const by = ext => arr.find(f => f.name.toLowerCase().endsWith(ext));
  const fm = by('.map'), fs = by('.stg'), fd = by('.dds'), fj = by('.json');
  try {
    if (fm) {
      const buf = new Uint8Array(await fm.arrayBuffer());
      const m = parseMap(buf);
      S.m = m; S.origTiles = m.tiles.slice(); S.tplBytes = buf; S.tplCrc = crc32(buf); S.tplName = fm.name; S.example = false; S.mini = null; S.undo = []; S.redo = []; S.sel = null; S.treeSel.clear(); S.dirtyForest = false;
      if (!fs) { S.stg = null; S.stgOrig = null; }
      const base = fm.name.replace(/\.map$/i, '');
      $('expName').value = BUILTIN_MAPS.some(b => b.toLowerCase() === base.toLowerCase()) ? base + '_Custom' : base; showNameHint();
      $('banner').hidden = true;
      $('subtitle').textContent = 'Editing ' + fm.name;
    }
    if (fs) {
      if (!S.m) throw new Error('Open the .map file together with the .stg file.');
      const s = parseStg(await fs.arrayBuffer());
      const maxX = Math.max(...s.troops.map(t => t.x), ...s.rects.map(r => Math.max(r.l, r.r)), 0);
      if (maxX > S.m.W * CELL + 1) toast('This stage file was made for a bigger map than the open one: some troops lie outside it.', 7000);
      S.stg = s; S.stgOrig = new Uint8Array(await fs.arrayBuffer()); S.stgCrc = crc32(S.stgOrig); S.stgName = fs.name;
      if (!$('waves').hidden) buildWaves();
    }
    if (fd) { try { S.mini = decodeDDS(await fd.arrayBuffer()); } catch (e) { toast('Minimap not used: ' + e.message); } }
    if (S.m) { computeTileColours(); fitView(); redrawAll(); refreshInfo(); buildTroopList(); buildToolOpts(); updUndo(); }
    if (fj) await loadProject(fj);
    const got = [fm && 'map', fs && 'stage', fd && 'minimap', fj && 'project'].filter(Boolean);
    if (got.length) toast('Opened ' + got.join(', ') + '.' + (fm && !fs ? ' Add the .stg to edit troops and spawn areas.' : '') + (fm && !fd ? ' Add the .dds for true ground colours.' : ''), 6000);
  } catch (e) { toast(e.message, 8000); }
}
$('bOpen').onclick = () => $('fileIn').click();
$('fileIn').onchange = e => { openFiles(e.target.files); e.target.value = ''; };
const stage = $('stage');
stage.addEventListener('dragover', e => { e.preventDefault(); $('drop').hidden = false; });
stage.addEventListener('dragleave', e => { if (!e.relatedTarget || !stage.contains(e.relatedTarget)) $('drop').hidden = true; });
stage.addEventListener('pointerdown', () => { $('drop').hidden = true; }, true);
stage.addEventListener('drop', e => { e.preventDefault(); $('drop').hidden = true; openFiles(e.dataTransfer.files); });

async function saveFile(name, data) {
  if (!downloadsNS && window.claude && window.claude.use) {   /* inside the artifact viewer: wait for the save capability once */
    try { downloadsNS = await Promise.race([window.claude.use('downloads'), new Promise(r => setTimeout(() => r(null), 3000))]); } catch (_) { downloadsNS = null; }
    if (!downloadsNS) { showNoSave(); return; }
  }
  if (downloadsNS) {
    try { await downloadsNS.save({ filename: name, data: new Blob([data]) }); toast('Saved ' + name + '.'); return; }
    catch (e) { if (e && e.code === 'declined') { toast('Save cancelled.'); return; } if (e && e.code === 'rate_limited') { toast('A save is already waiting for your answer.'); return; } }
  }
  const url = URL.createObjectURL(new Blob([data])), a = document.createElement('a');
  a.href = url; a.download = name; document.body.append(a); a.click(); a.remove(); setTimeout(() => URL.revokeObjectURL(url), 4000);
  toast('Saved ' + name + '.');
}
function showNoSave() {
  const b = $('banner');
  b.innerHTML = 'Saving files is not available in this online preview for your account. Use the <b>Bersia Map Forge</b> HTML file instead: download it from the Battle for Bersia MP Discord or GitHub, open it in Chrome, Edge or Firefox, and everything here works, saving included.';
  b.hidden = false;
  toast('Saving is not available here - see the note at the top of the map.', 7000);
}
function minimapRGBA() {
  const m = S.m, N = 512, out = new Uint8ClampedArray(N * N * 4);
  if (needBase) { renderBase(); needBase = false; }
  const base = baseCv.getContext('2d').getImageData(0, 0, m.W, m.H).data;
  const treeAt = new Uint8Array(m.W * m.H);
  for (const p of m.props) if (isTreeLike(p.id)) { const c = Math.floor(p.x / CELL), r = Math.floor(m.H - p.y / CELL); if (c >= 0 && r >= 0 && c < m.W && r < m.H) treeAt[r * m.W + c] = 1; }
  for (let y = 0; y < N; y++) for (let x = 0; x < N; x++) {
    const c = Math.min(m.W - 1, (x * m.W / N) | 0), r = Math.min(m.H - 1, (y * m.H / N) | 0), i = r * m.W + c, q = (y * N + x) * 4;
    let R = base[i * 4], G = base[i * 4 + 1], B = base[i * 4 + 2];
    const a = m.attr[attrIndex(r, c)];
    if (treeAt[i] || (a & 0x80)) { R = R * 0.55; G = G * 0.68; B = B * 0.55; }
    if (a & 0x40) { R *= 0.6; G *= 0.6; B *= 0.6; }
    out[q] = R; out[q + 1] = G; out[q + 2] = B; out[q + 3] = 255;
  }
  return out;
}
/* the lobby lists a .stg in Data\Mission\Live as "Custom: <name>" when the name is at most 27 characters and is not one of the
   game's own 27 maps (net.c cm_scan; up to 8 custom maps) */
const BUILTIN_MAPS = ['4_Colonock','4_ForestDubine','4_Posterus','4_ToukenCliff','6_AgerSanguini','6_Blackedmark','6_BronzeWood','6_BurialGround','6_CloudBourne','6_DangerZone','6_DesertDowns','6_DryGlade','6_Gremium','6_Greywood','6_Halmoral','6_Mushroom','6_NorthWind','6_Norwart','6_Nymphbarren','6_Osmond','6_PathFinder','6_Posterus','6_TurtleBeach','6_Whitewolf','8_Ruin','9_IceStorm','9_Plains'];
const cleanName = v => v.trim().replace(/\.(map|stg|dds|zip)$/i, '').replace(/[^A-Za-z0-9_\-]/g, '');
function nameProblem(n) {
  if (!n) return 'Type a name for your map.';
  if (BUILTIN_MAPS.some(b => b.toLowerCase() === n.toLowerCase())) return `"${n}" is one of the game's own maps - exporting under that name would replace it, and the lobby would not list it as a custom map. Pick a new name, e.g. ${n}_2.`;
  if (n.length > 27) return 'Use 27 characters or fewer.';
  return '';
}
function showNameHint() {
  const raw = $('expName').value, n = cleanName(raw), pr = nameProblem(n), h = $('nameHint');
  if (pr) { h.textContent = pr; h.className = 'namehint warn'; }
  else { h.textContent = (n !== raw.trim() ? `Spaces and other symbols are left out: the map will be saved as "${n}". ` : '') + `In the lobby: Custom: ${n}`; h.className = 'namehint note'; }
}
$('expName').addEventListener('input', showNameHint);
function exportName() { return cleanName($('expName').value); }
$('bExport').onclick = async () => {
  if (!S.m) return;
  const m = S.m, name = exportName(), pr = nameProblem(name);
  if (pr) { showNameHint(); $('expName').focus(); toast('Not exported: ' + pr, 8000); return; }
  if (S.example) toast('Note: this is the made-up example map. To make a real map, open one from your game folder first (Help explains how).', 8000);
  else if (!S.stg) toast('Exported without a .stg file: the map has no troops and won\'t show in the lobby. Open the map\'s .stg as well and export again.', 9000);
  if (S.dirtyForest) rebuildForestGrids(m);
  const mapBytes = buildMap(m), dds = encodeDDS(minimapRGBA(), 512, 512);
  const files = [{ name: 'Data/Mission/Live/' + name + '.map', data: mapBytes }, { name: 'Data/Mission/Live/' + name + '.dds', data: dds }];
  if (S.stg) files.push({ name: 'Data/Mission/Live/' + name + '.stg', data: buildStg(S.stg, name + '.map', name + '.bmp') });
  const readme = `${name} - made with Bersia Map Forge\r\n\r\nHow to install: copy the Data folder from this zip into your Kingdom Under Fire: Heroes game folder (the folder with Heroes.exe).\r\nThe files go into Data\\Mission\\Live. In the multiplayer lobby the map is listed as "Custom: ${name}".\r\nOnly the host needs it: players who join without it get it from the host automatically (Battle for Bersia MP 1.5 and later).\r\n` + (S.stg ? '' : '\r\nThis export has no .stg stage file: open one in the editor to include player starts and spawn areas.\r\n');
  files.push({ name: 'README.txt', data: new TextEncoder().encode(readme) });
  await saveFile(name + '.zip', makeZip(files));
};

/* ---------- project files (your changes only, applied to your own copy of the original map) ---------- */
async function gz(u8) { if (typeof CompressionStream === 'undefined') return { z: false, b: u8 }; const s = new Blob([u8]).stream().pipeThrough(new CompressionStream('gzip')); return { z: true, b: new Uint8Array(await new Response(s).arrayBuffer()) }; }
async function ungz(u8, z) { if (!z) return u8; const s = new Blob([u8]).stream().pipeThrough(new DecompressionStream('gzip')); return new Uint8Array(await new Response(s).arrayBuffer()); }
const b64 = u8 => { let s = ''; for (let i = 0; i < u8.length; i += 0x8000) s += String.fromCharCode.apply(null, u8.subarray(i, i + 0x8000)); return btoa(s); };
const unb64 = s => Uint8Array.from(atob(s), c => c.charCodeAt(0));
$('bProject').onclick = async () => {
  if (!S.m) return;
  const m = S.m, fromTpl = !!S.tplBytes && !S.example;
  const orig = fromTpl ? parseMap(S.tplBytes) : null;
  const W = m.W, H = m.H, nT = W * H, nH = (W + 1) * (H + 1);
  const blob = new Uint8Array(nT * 2 + nH * 2 + m.attr.length), dv = new DataView(blob.buffer);
  for (let i = 0; i < nT; i++) dv.setUint16(i * 2, (m.tiles[i] ^ (orig ? orig.tiles[i] : 0)) & 0xffff, true);
  for (let i = 0; i < nH; i++) dv.setInt16(nT * 2 + i * 2, m.heights[i] - (orig ? orig.heights[i] : 0), true);
  for (let i = 0; i < m.attr.length; i++) blob[nT * 2 + nH * 2 + i] = m.attr[i] ^ (orig ? orig.attr[i] : 0);
  const z = await gz(blob);
  const keep = [], added = [];
  for (const p of m.props) { if (orig && p.orig != null) keep.push(p.orig); else added.push([p.id, p.inst, +p.x.toFixed(2), +p.y.toFixed(2), p.rot >>> 0]); }
  const proj = { format: 'bersia-map-forge', version: 1, base: fromTpl ? { file: S.tplName, crc: S.tplCrc, W, H } : { file: null, W, H }, tileset: m.tileset,
    grids: { z: z.z, data: b64(z.b) }, keepProps: orig ? keep : null, addProps: added,
    stage: S.stg && S.stgOrig ? { file: S.stgName, crc: S.stgCrc, ops: diffBytes(S.stgOrig, buildStg(S.stg)) } : null, name: exportName() };
  await saveFile(exportName() + '.mapforge.json', JSON.stringify(proj));
};
async function loadProject(file) {
  const p = JSON.parse(await file.text());
  if (p.format !== 'bersia-map-forge') throw new Error('This JSON file is not a Map Forge project.');
  let m;
  if (p.base.file) {
    if (!S.tplBytes || S.tplCrc !== p.base.crc) throw new Error('This project was made from ' + p.base.file + '. Open that original map file from your own game folder together with the project.');
    m = parseMap(S.tplBytes); S.origTiles = m.tiles.slice();
  } else m = newMap(p.base.W, Math.max(0, tilesetIndex(p.tileset)), 0);
  const W = m.W, H = m.H, nT = W * H, nH = (W + 1) * (H + 1);
  const blob = await ungz(unb64(p.grids.data), p.grids.z), dv = new DataView(blob.buffer);
  for (let i = 0; i < nT; i++) m.tiles[i] ^= dv.getUint16(i * 2, true);
  for (let i = 0; i < nH; i++) m.heights[i] += dv.getInt16(nT * 2 + i * 2, true);
  for (let i = 0; i < m.attr.length; i++) m.attr[i] ^= blob[nT * 2 + nH * 2 + i];
  if (p.tileset !== m.tileset) { m.tileset = p.tileset; m.tilesetChanged = true; }
  const props = p.keepProps ? p.keepProps.map(i => m.props[i]) : [];
  let hz = false; for (let i = 0; i < nH; i++) if (dv.getInt16(nT * 2 + i * 2, true)) { hz = true; break; }
  if (hz) for (const q of props) q.h = groundHeight(m, q.x, q.y);   /* the editor re-grounds every prop after a height stroke */
  for (const a of p.addProps) props.push({ id: a[0], inst: a[1], x: a[2], y: a[3], rot: a[4], h: 0 });
  for (const q of props) if (q.h === 0 && q.orig == null) q.h = groundHeight(m, q.x, q.y);
  m.props = props;
  let fz = false; for (let i = 0; i < m.attr.length; i++) if (blob[nT * 2 + nH * 2 + i] & 0x90) { fz = true; break; }
  S.m = m; S.dirtyForest = fz || (p.addProps && p.addProps.length > 0);
  if (p.stage && p.stage.ops && S.stgOrig && S.stgCrc === p.stage.crc) { S.stg = parseStg(patchBytes(S.stgOrig, p.stage.ops)); }
  else if (p.stage) toast('Open the original ' + (p.stage.file || '.stg') + ' with the project to get the troop, area and wave changes back.', 7000);
  if (p.name) { $('expName').value = p.name; showNameHint(); }
  S.undo = []; S.redo = []; updUndo();
  computeTileColours(); fitView(); redrawAll(); refreshInfo(); buildTroopList(); buildToolOpts();
}

/* ---------- new map ---------- */
$('bNew').onclick = () => $('dlgNew').showModal();
$('dlgNew').addEventListener('close', () => {
  if ($('dlgNew').returnValue !== 'ok') return;
  const W = +$('nSize').value, ts = +$('nTile').value, h = Math.max(0, Math.min(6375, +$('nHeight').value || 0));
  if (S.m) pushUndo();
  const keepStg = S.stg && Math.max(...S.stg.troops.map(t => t.x), 0) <= W * CELL;
  S.m = newMap(W, ts, h); S.origTiles = null; S.tplBytes = null; S.tplName = ''; S.example = false; S.mini = null; S.treeSel.clear(); S.dirtyForest = false;
  if (S.stg && !keepStg) { S.stg = null; toast('The open stage file belongs to a bigger map, so it was closed.'); }
  S.undo = []; S.redo = []; updUndo();
  $('banner').hidden = true; $('subtitle').textContent = 'New ' + TILESET_NAMES[ts].toLowerCase() + ' map';
  computeTileColours(); fitView(); redrawAll(); refreshInfo(); buildTroopList(); buildToolOpts();
});
$('tileset').onchange = () => {
  if (!S.m) return; pushUndo();
  const i = +$('tileset').value; S.m.tileset = TILESETS[i]; S.m.tilesetChanged = true; S.mini = null;
  computeTileColours(); redrawAll(); buildToolOpts();
  toast('Ground type set to ' + TILESET_NAMES[i] + '. Tiles keep their numbers, so check paths and edges in the game.');
};
function fillSelects() {
  for (const id of ['tileset', 'nTile']) {
    const s = $(id); s.innerHTML = '';
    TILESET_NAMES.forEach((n, i) => { if (i === 5 || i === 6) return; const o = document.createElement('option'); o.value = i; o.textContent = n; s.append(o); });
  }
  $('nTile').value = '1';
}

/* ---------- example map (made up here, no game data) ---------- */
function exampleMap() {
  const m = newMap(200, 1, 0, 0x51ED), W = 200, rnd = rng(77);
  const s = W + 1;
  const bumps = []; for (let i = 0; i < 14; i++) bumps.push([rnd() % 200, rnd() % 200, 12 + rnd() % 26, 120 + rnd() % 380]);
  for (let r = 0; r <= W; r++) for (let c = 0; c <= W; c++) {
    let h = 160;
    for (const [bx, by, rad, amp] of bumps) { const d = Math.hypot(c - bx, r - by) / rad; if (d < 1) h += amp * (1 - d * d) ** 2; }
    const ridge = Math.abs(c - (70 + Math.sin(r / 23) * 18)); if (ridge < 4 && r > 40 && r < 150) h += (4 - ridge) * 140;
    m.heights[r * s + c] = Math.round(h);
  }
  for (let i = 0; i < W * W; i++) { const r = (i / W) | 0, c = i % W; if (Math.abs(c - 120 - Math.sin(r / 15) * 10) < 2.2) m.tiles[i] = (40 + (rnd() & 3)) | ((rnd() & 3) << 14); }
  const trees = [57, 58, 59];
  const blobs = [[150, 60, 18], [40, 150, 22], [165, 160, 14]];
  let inst = 1;
  for (let r = 0; r < W; r++) for (let c = 0; c < W; c++) {
    let inF = false; for (const [bx, by, rad] of blobs) if (Math.hypot(c - bx, r - by) < rad * (0.8 + 0.3 * Math.sin(c * 0.7 + r))) inF = true;
    const k = (W - 1 - r) * W + c;
    if (inF) { m.attr[k] |= 0x80; if (rnd() % 100 < 38) { const x = (c + 0.5) * CELL, y = (W - r - 0.5) * CELL; m.props.push({ id: trees[rnd() % 3], inst: inst++, x, y, h: 0, rot: ((rnd() & 15) << 20 | (rnd() & 15) << 16) >>> 0 }); m.attr[k] |= 0x10; } }
    const ridge = Math.abs(c - (70 + Math.sin(r / 23) * 18)); if (ridge < 1.5 && r > 40 && r < 150) m.attr[k] |= 0x40;
  }
  for (const p of m.props) p.h = groundHeight(m, p.x, p.y);
  rebuildForestGrids(m);
  return m;
}

/* ---------- change-only byte diff (projects never carry the original stage file) ---------- */
function diffBytes(a, b) {
  const K = 16, idx = new Map(), ops = [];
  const key = (u, i) => { let h = 2166136261; for (let k = 0; k < K; k++) h = Math.imul(h ^ u[i + k], 16777619); return h >>> 0; };
  for (let i = 0; i + K <= a.length; i++) { const h = key(a, i); const l = idx.get(h); if (!l) idx.set(h, [i]); else if (l.length < 8) l.push(i); }
  let i = 0, lit = [], lastEnd = 0;
  const flush = () => { if (lit.length) { ops.push([1, b64(Uint8Array.from(lit))]); lit = []; } };
  while (i < b.length) {
    let best = -1, bl = 0;
    if (i + K <= b.length) {
      const cands = (idx.get(key(b, i)) || []).slice();
      if (lastEnd + K <= a.length) cands.unshift(lastEnd);
      for (const c of cands) { let n = 0; while (c + n < a.length && i + n < b.length && a[c + n] === b[i + n]) n++; if (n > bl) { bl = n; best = c; } }
    }
    if (bl >= K) { flush(); ops.push([0, best, bl]); i += bl; lastEnd = best + bl; } else { lit.push(b[i]); i++; }
  }
  flush();
  return ops;
}
function patchBytes(a, ops) {
  const parts = ops.map(o => o[0] === 0 ? a.subarray(o[1], o[1] + o[2]) : unb64(o[1]));
  const out = new Uint8Array(parts.reduce((n, x) => n + x.length, 0)); let p = 0;
  for (const x of parts) { out.set(x, p); p += x.length; }
  return out;
}

/* ---------- Invasion waves ---------- */
const JOBNAME = ['Archer','Longbow','Infantry','Spearmen','Heavy Infantry','Knight','Paladin','Cavalry','Heavy Cavalry','Storm Rider','Sapper','Pyrotechnician','Bomber Wing','Mortar','Ballista','Ballista (2)','Catapult','Battaloon',
  'Dark Elf Archer','Cavalry Archer','Dark Elf Fighter','Dark Knight','Dark Cavalry','Orc Infantry','Orc Rider','Orc Heavy Rider','Orc Axemen','Orc Heavy','Orc Sapper','Scorpion','Swamp Mammoth','Dirigible','Black Wyvern','Ghoul','Bone Dragon',
  'Wall Guards','Scout','Exploding Ghoul','Encablossa','Encablossa Flyer','Encablossa Spitter','Dark Wall Guards','Encablossa (2)',
  'Earth Golem','Ice Maiden','Thunder Rhino','Flame Wraith','Earth Golem (dark 1)','Ice Maiden (dark 1)','Thunder Rhino (dark 1)','Flame Wraith (dark 1)','Earth Golem (dark 2)','Ice Maiden (dark 2)','Thunder Rhino (dark 2)','Flame Wraith (dark 2)',
  'Encablossa Infested','Dwarf','Elf Warrior','Dark Elf Guard','Lich','Ogre','Encablossa (E build)','Encablossa Spitter (E build)','Encablossa Infested (E build)','Encablossa Flyer (E build)'];
/* colour per kind of troop, so a wave can be read at a glance */
const JOBCAT = [['Bows', '#7bd389', [0, 1, 18, 19]], ['Foot', '#d7a35e', [2, 3, 4, 20, 23, 26, 27, 35, 41, 56, 57, 58]], ['Horse', '#5aa9e8', [5, 6, 7, 8, 21, 22, 24, 25]],
  ['Air', '#9ad0d0', [9, 12, 31, 32, 34, 39, 64]], ['Siege & support', '#e8735a', [10, 11, 13, 14, 15, 16, 17, 28, 29, 30]], ['Monsters & elementals', '#d98ae0', []]];
const jobCat = j => JOBCAT.find(c => c[2].includes(j)) || JOBCAT[5];
const jobName = j => JOBNAME[j] || ('Job ' + j);
const tLabel = t => jobName(t.job) + ' #' + t.id;
S.ai = {}; S.waveFilter = 0; S.importPool = []; S.importName = '';
function parseAI(text) {
  const out = {}, re = /TROOP_AI_DEF([\s\S]*?)(?=TROOP_AI_DEF|$)/g; let m;
  const human = tok => tok.replace(/^AI_MODE_/, '').replace(/^ABILITY_/, '').toLowerCase().replace(/_/g, ' ');
  while ((m = re.exec(text))) {
    const body = m[1].replace(/\/\/[^\n]*/g, ''), id = /\bID\s+0*(\d+)/.exec(body); if (!id) continue;
    const name = /\bNAME\s+"([^"]*)"/.exec(body);
    const modes = [...new Set((body.match(/\bAI_MODE_[A-Z0-9_]+/g) || []).map(human))].slice(0, 3);
    const abil = [...new Set((body.match(/\bABILITY_[A-Z0-9_]+/g) || []).filter(x => !/DELAY/.test(x)).map(human))].slice(0, 2);
    const sum = [modes.join(', '), abil.length ? 'uses ' + abil.join(', ') : ''].filter(Boolean).join('; ');
    out[+id[1]] = { name: name ? name[1] : '', sum: sum || 'no orders' };
  }
  return out;
}
function wavePlayer(w) {
  const c = new Map();
  for (const l of w.lines) for (const id of lineGet(l).troops) { const t = S.stg.troops.find(x => x.id === id); if (t) c.set(t.player, (c.get(t.player) || 0) + 1); }
  if (c.size) return [...c.entries()].sort((a, b) => b[1] - a[1])[0][0];
  const p = S.stg.playerType.findIndex(x => x === 3); return p < 0 ? 1 : p;
}
function newWaveTroop(src, w) {
  const wp = wavePlayer(w), ref = S.stg.troops.find(t => t.player === wp) || S.stg.troops[0];
  const t = addTroop(S.stg, src, null);
  t.raw[0x24] = wp; t.raw[0x25] = 0; t.raw[0x26] = 0; t.raw[0x27] = 0; t.raw[0x5a] = 0xff;
  refreshTroop(t); t.x = ref.x; t.y = ref.y; t.dir = ref.dir;
  return t;
}
function el(tag, attrs, ...kids) {
  const e = document.createElement(tag);
  for (const [k, v] of Object.entries(attrs || {})) { if (k === 'class') e.className = v; else if (k.startsWith('on')) e[k] = v; else if (k === 'text') e.textContent = v; else e.setAttribute(k, v); }
  for (const k of kids) if (k != null) e.append(k);
  return e;
}
function numIn(id, val, min, max, onchange) {
  const i = el('input', { type: 'number', id, min, max, step: 1, class: 'wv-num' }); i.value = val;
  i.onchange = () => { let v = Math.round(+i.value); if (!isFinite(v)) v = min; v = Math.max(min, Math.min(max, v)); pushUndo(); onchange(v); buildWaves(); };
  return i;
}
function chipFor(t, id) {
  const c = el('span', { class: 'wv-chip', role: 'button', tabindex: '0', title: t ? `${jobName(t.job)} · ${t.w * t.d} soldiers · leader level ${t.level || '?'} · troop #${t.id}${t.name ? ' "' + t.name + '"' : ''}. Click to find it in Wave troops.` : 'This troop is missing from the stage file' });
  if (t) { c.append(el('span', { class: 'cd', style: 'background:' + jobCat(t.job)[1] }), el('span', { text: jobName(t.job) }), el('span', { class: 'sm', text: '#' + t.id + ' · ' + t.w * t.d })); }
  else c.append(el('span', { text: 'missing #' + id }));
  const go = () => { const r = document.getElementById('wk' + (t ? t.idx : -1)); if (!r) return; const row = r.closest('.wv-row'); row.scrollIntoView({ block: 'center', behavior: 'smooth' }); row.classList.add('flash'); setTimeout(() => row.classList.remove('flash'), 1600); r.focus({ preventScroll: true }); };
  c.onclick = e => { if (e.target.tagName !== 'BUTTON') go(); };
  c.onkeydown = e => { if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); go(); } };
  return c;
}
function legend() {
  const l = el('div', { class: 'wv-legend', 'aria-label': 'Troop colours' });
  for (const [n, col] of JOBCAT) l.append(el('span', {}, el('span', { class: 'cd', style: `width:8px;height:8px;border-radius:50%;background:${col}` }), n));
  l.append(el('span', { text: '· the number after # is the troop\'s id, then its soldiers' }));
  return l;
}
function buildWaves() {
  const box = $('wavesBody'); box.innerHTML = '';
  if (!S.stg) { box.append(el('p', { class: 'note', text: 'Open an Invasion map first: press Open map files… and pick 8_Ruin or 9_IceStorm (.map, .stg and .dds) from Data\\Mission\\Live in your game folder.' })); return; }
  const w = waveModel(S.stg);
  if (!w.lines.length && !w.begin) { box.append(el('p', { class: 'note', text: 'This map has no Invasion waves - it is a Versus map. Only Invasion maps (8_Ruin, 9_IceStorm and maps made from them) have waves. To make an Invasion map, start from one of those two.' })); return; }
  const lines = w.lines.map(l => ({ l, ...lineGet(l) }));
  const byId = new Map(S.stg.troops.map(t => [t.id, t]));
  const maxN = w.param ? pInt(w.param.params[6]) : 0, begin = w.begin ? pInt(w.begin.params[0]) : 0;
  const lastWave = Math.max(1, ...lines.map(x => x.to));

  /* guide */
  const guide = el('details', { class: 'wv-card wv-guide' });
  try { if (localStorage.getItem('bmf.wvGuide') !== 'closed') guide.open = true; } catch (_) { guide.open = true; }
  guide.ontoggle = () => { try { localStorage.setItem('bmf.wvGuide', guide.open ? 'open' : 'closed'); } catch (_) {} };
  const ol = el('ol');
  [['Groups. ', 'Each row under Wave groups is one group of enemy troops. A group attacks in every wave from its first wave to its last (1 – 1 means wave 1 only; 3 – 10 means waves 3 to 10), at the level you give it, with the AI orders you pick.'],
   ['Adding troops. ', 'Use + troop at the end of a row: pick an enemy troop that is not used yet, or "Make a new troop like…" to add a fresh copy of any kind. × removes a troop from that group (the troop stays in the map).'],
   ['Finding a troop. ', 'Every troop has a colour for its kind (see the key), its id after # and its number of soldiers. Click it to jump to it in Wave troops, where you can turn it into another kind of troop.'],
   ['Where they arrive. ', 'Each wave comes out of one of the spawn areas in Settings, picked at random. Drag the blue boxes on the map (Back to the map) to move them.'],
   ['Before you export, ', 'fix anything listed in the orange Check these box. The bar chart shows how many troops each wave has - red bars have more than can be out at once.']]
    .forEach(([b, t]) => ol.append(el('li', {}, el('b', { text: b }), t)));
  guide.append(el('summary', { text: 'How waves work (click to show or hide)' }), ol);
  box.append(guide);

  /* settings */
  const set = el('div', { class: 'wv-card' }, el('h3', { text: 'Settings' }));
  const r1 = el('div', { class: 'row' });
  if (w.begin) r1.append(el('label', { for: 'wvBegin', text: 'Number of waves', title: 'How many waves the battle has' }), numIn('wvBegin', begin, 1, 999, v => setPInt(w.begin.params[0], v)));
  if (w.param) r1.append(el('label', { for: 'wvMax', text: 'Most enemy troops out at once', title: 'If a wave has more troops than this, the rest wait until some are beaten' }), numIn('wvMax', maxN, 1, 230, v => setPInt(w.param.params[6], v)));
  set.append(r1);
  if (w.param) {
    const r2 = el('div', { class: 'row' }, el('span', { class: 'note', text: 'Spawn areas (each wave picks one of these at random; "none" = slot not used):' }));
    for (let k = 0; k < 6; k++) {
      const sel = el('select', { id: 'wvArea' + k, 'aria-label': 'Spawn area ' + (k + 1) });
      sel.append(el('option', { value: '-1', text: 'none' }));
      for (const a of S.stg.rects) sel.append(el('option', { value: String(a.id), text: 'Area ' + a.id }));
      sel.value = String(pInt(w.param.params[k]));
      sel.onchange = () => { pushUndo(); setPInt(w.param.params[k], +sel.value); buildWaves(); needDraw = true; };
      r2.append(sel);
    }
    set.append(r2, el('p', { class: 'note', text: 'The areas are the blue boxes on the map, labelled A0, A1… Press Back to the map to drag or resize them, or to add a new one (Select tool → Add an area).' }));
  }
  box.append(set);

  /* chart: troops per wave */
  const counts = [], lv = [];
  for (let n = 1; n <= lastWave; n++) { const ls = lines.filter(x => x.from <= n && n <= x.to); counts.push(ls.reduce((s, x) => s + x.troops.length, 0)); lv.push(ls.length ? Math.round(ls.reduce((s, x) => s + x.level, 0) / ls.length) : 0); }
  const top = Math.max(1, ...counts, maxN && maxN < 60 ? maxN : 0);
  const chart = el('div', { class: 'wv-card' }, el('h3', { text: 'Troops per wave' }), el('p', { class: 'note', text: 'Bar height = number of troops in the wave; the small number above = their average level; the number below = the wave. Click a wave to show only its groups; click it again to show all.' }));
  const bars = el('div', { class: 'wv-bars', role: 'list' });
  counts.forEach((c, i) => {
    const n = i + 1, over = maxN && c > maxN;
    const b = el('button', { type: 'button', class: 'wv-bar' + (S.waveFilter === n ? ' on' : '') + (over ? ' over' : ''), title: `Wave ${n}: ${c} troops, average level ${lv[i]}` + (over ? ` (only ${maxN} can be out at once)` : ''), 'aria-label': `Wave ${n}, ${c} troops` });
    b.append(el('span', { class: 'wv-lv', text: lv[i] ? String(lv[i]) : '' }), el('span', { class: 'wv-fill', style: `height:${Math.round(100 * c / top)}%` }), el('span', { class: 'wv-n', text: String(n) }));
    b.onclick = () => { S.waveFilter = S.waveFilter === n ? 0 : n; buildWaves(); };
    bars.append(b);
  });
  chart.append(bars);
  box.append(chart);

  /* warnings */
  const warn = [];
  const seen = new Map();
  lines.forEach((x, li) => {
    if (!x.troops.length) warn.push(`Group ${li + 1} has no troops: add some with + troop, or delete the group.`);
    if (x.from > x.to) warn.push(`Group ${li + 1} runs from wave ${x.from} to ${x.to}, so it never comes. Make the first wave the smaller number.`);
    if (begin && x.from > begin) warn.push(`Group ${li + 1} starts at wave ${x.from}, after the last wave (${begin}). Raise Number of waves or lower its first wave.`);
    for (const id of x.troops) {
      if (!byId.has(id)) warn.push(`Group ${li + 1} lists troop #${id}, which is not in this map. Remove it with ×.`);
      for (const [lj, y] of seen.get(id) || []) if (Math.max(x.from, y.from) <= Math.min(x.to, y.to)) warn.push(`${byId.has(id) ? tLabel(byId.get(id)) : '#' + id} is in groups ${lj + 1} and ${li + 1} for the same waves: only group ${lj + 1} counts. Use a new troop (+ troop → Make a new troop like…) instead.`);
      if (!seen.has(id)) seen.set(id, []); seen.get(id).push([li, x]);
    }
  });
  counts.forEach((c, i) => { if (maxN && c > maxN) warn.push(`Wave ${i + 1} has ${c} troops but only ${maxN} can be out at once - the rest wait until some are beaten.`); });
  if (S.stg.troops.length > 230) warn.push(`The map has ${S.stg.troops.length} troops; the game allows 230 at most. Remove some troops from groups and delete unused ones.`);
  if (warn.length) { const wc = el('div', { class: 'wv-card wv-warn' }, el('h3', { text: 'Check these' })); const ul = el('ul'); for (const x of warn.slice(0, 12)) ul.append(el('li', { text: x })); if (warn.length > 12) ul.append(el('li', { text: `…and ${warn.length - 12} more.` })); wc.append(ul); box.append(wc); }

  /* groups */
  const aiIds = [...new Set([...lines.map(x => x.ai), ...Object.keys(S.ai).map(Number)])].sort((a, b) => a - b);
  const aiLabel = id => S.ai[id] ? `${id} · ${S.ai[id].sum}` : `AI ${id}`;
  const enemy = S.stg.troops.filter(t => (S.stg.playerType[t.player] || 0) === 3);
  const lc = el('div', { class: 'wv-card' }, el('h3', { text: S.waveFilter ? `Groups in wave ${S.waveFilter}` : 'Wave groups' }),
    el('p', { class: 'note', text: 'One row = one group of troops that attacks in every wave from its first to its last, at its level and with its AI orders.' + (Object.keys(S.ai).length ? '' : ' AI orders: if unsure, keep the number the group already has. "Explain the AI numbers…" (top) shows what each number does.') }), legend());
  const tbl = el('div', { class: 'wv-table', role: 'table' });
  tbl.append(el('div', { class: 'wv-row wv-th', role: 'row' }, ...['Waves (first – last)', 'Level', 'AI orders', 'Troops', ''].map(h => el('span', { role: 'columnheader', text: h }))));
  lines.forEach((x, li) => {
    if (S.waveFilter && !(x.from <= S.waveFilter && S.waveFilter <= x.to)) return;
    const row = el('div', { class: 'wv-row', role: 'row' });
    const wv = el('span', { class: 'wv-cell' }, el('span', { class: 'wv-tag', text: (li + 1) + '.' }), numIn('wf' + li, x.from, 1, 999, v => lineSet(x.l, { from: v })), el('span', { class: 'note', text: '–' }), numIn('wt' + li, x.to, 1, 999, v => lineSet(x.l, { to: v })));
    wv.querySelector('#wf' + li).setAttribute('aria-label', `Group ${li + 1}: first wave`); wv.querySelector('#wt' + li).setAttribute('aria-label', `Group ${li + 1}: last wave`);
    const lvl = el('span', { class: 'wv-cell' }, numIn('wl' + li, x.level, 1, 99, v => lineSet(x.l, { level: v })));
    lvl.querySelector('input').setAttribute('aria-label', `Group ${li + 1}: level`);
    const ai = el('select', { id: 'wa' + li, 'aria-label': 'AI orders for group ' + (li + 1) });
    for (const id of aiIds) ai.append(el('option', { value: String(id), text: aiLabel(id) }));
    ai.value = String(x.ai); ai.onchange = () => { pushUndo(); lineSet(x.l, { ai: +ai.value }); buildWaves(); };
    const chips = el('span', { class: 'wv-chips' });
    x.troops.forEach((id, k) => {
      const t = byId.get(id), c = chipFor(t, id);
      c.append(el('button', { type: 'button', 'aria-label': 'Remove ' + (t ? tLabel(t) : id) + ' from this group', title: 'Remove from this group', text: '×', onclick: () => { pushUndo(); const tr = x.troops.slice(); tr.splice(k, 1); lineSet(x.l, { troops: tr }); buildWaves(); } }));
      chips.append(c);
    });
    if (x.troops.length < 10) {
      const add = el('select', { id: 'wn' + li, 'aria-label': 'Add a troop to group ' + (li + 1), class: 'wv-add' });
      add.append(el('option', { value: '', text: '+ troop' }));
      const used = new Set(lines.flatMap(y => y.troops));
      const og3 = el('optgroup', { label: 'Make a new troop like…' }), og1 = el('optgroup', { label: 'Enemy troops not in any group yet' }), og2 = el('optgroup', { label: 'Enemy troops already in another group' });
      for (const t of enemy) if (!x.troops.includes(t.id)) (used.has(t.id) ? og2 : og1).append(el('option', { value: 'id:' + t.id, text: tLabel(t) + ' · ' + t.w * t.d + ' soldiers' }));
      const kinds = new Map(); for (const t of S.stg.troops.concat(S.importPool)) { const k = t.job + ':' + t.unit + ':' + t.w * t.d; if (!kinds.has(k) && t.job >= 0 && t.unit >= 0) kinds.set(k, t); }
      for (const [k, t] of [...kinds].sort((a, b) => jobName(a[1].job).localeCompare(jobName(b[1].job)))) og3.append(el('option', { value: 'new:' + (S.importPool.includes(t) ? 'i' + S.importPool.indexOf(t) : t.idx), text: jobName(t.job) + ' · ' + t.w * t.d + ' soldiers' + (S.importPool.includes(t) ? ' (from ' + S.importName + ')' : '') }));
      [og3, og1, og2].forEach(g => { if (g.children.length) add.append(g); });
      add.onchange = () => {
        const v = add.value; if (!v) return; pushUndo();
        let id;
        if (v.startsWith('id:')) id = +v.slice(3);
        else { const key = v.slice(4), src = key[0] === 'i' ? S.importPool[+key.slice(1)] : S.stg.troops[+key]; id = newWaveTroop(src, w).id; }
        lineSet(x.l, { troops: x.troops.concat([id]) }); buildWaves(); buildTroopList(); needDraw = true;
        toast('Added ' + tLabel(byId.get(id) || S.stg.troops.find(t => t.id === id)) + ' to group ' + (li + 1) + '.');
      };
      chips.append(add);
    } else chips.append(el('span', { class: 'note', text: 'full (10 troops)' }));
    const del = el('button', { type: 'button', class: 'btn wv-del', text: 'Delete', title: 'Delete this group (its troops stay in the map)', 'aria-label': 'Delete group ' + (li + 1), onclick: () => { pushUndo(); removeWaveLine(w, x.l); buildWaves(); toast(`Group ${li + 1} deleted. Ctrl+Z brings it back.`); } });
    row.append(wv, lvl, el('span', { class: 'wv-cell' }, ai), chips, el('span', { class: 'wv-cell' }, del));
    tbl.append(row);
  });
  lc.append(tbl);
  lc.append(el('div', { class: 'row' }, el('button', { type: 'button', class: 'btn', text: S.waveFilter ? `Add a group for wave ${S.waveFilter}` : 'Add a group (for the next wave)', onclick: () => {
    pushUndo(); const n = S.waveFilter || lastWave + 1;
    addWaveLine(S.stg, w, { from: n, to: n, level: Math.max(1, lv[n - 2] || lv[lv.length - 1] || 1), ai: lines.length ? lines[lines.length - 1].ai : 1 }); buildWaves();
    toast(`Added an empty group for wave ${n}. Add troops to it with + troop.`);
  } })));
  box.append(lc);

  /* troop kinds */
  const tk = el('div', { class: 'wv-card' }, el('h3', { text: 'Wave troops' }), el('p', { class: 'note', text: 'Every enemy troop that can come in a wave. To change what a troop is, pick a kind under "Turn it into…": it copies that kind\'s job, look, soldiers and leader, and the troop keeps its place in the groups. "Borrow troop types from another map…" (top) adds the kinds from any other .stg, such as a campaign mission.' }), legend());
  const list = el('div', { class: 'wv-table' });
  list.append(el('div', { class: 'wv-row wv-row2 wv-th' }, ...['Troop', 'Soldiers', 'In waves', 'Turn it into…'].map(h => el('span', { text: h }))));
  const inWaves = id => lines.filter(x => x.troops.includes(id)).map(x => x.from === x.to ? String(x.from) : x.from + '–' + x.to).join(', ');
  const srcKinds = new Map(); for (const t of S.stg.troops.concat(S.importPool)) { const k = t.job + ':' + t.unit + ':' + t.w * t.d; if (!srcKinds.has(k) && t.job >= 0 && t.unit >= 0) srcKinds.set(k, t); }
  const kindsSorted = [...srcKinds].sort((a, b) => jobName(a[1].job).localeCompare(jobName(b[1].job)));
  for (const t of enemy) {
    const sel = el('select', { 'aria-label': 'Turn ' + tLabel(t) + ' into another kind', id: 'wk' + t.idx });
    sel.append(el('option', { value: '', text: 'keep as it is' }));
    for (const [k, s] of kindsSorted) if (s !== t) sel.append(el('option', { value: S.importPool.includes(s) ? 'i' + S.importPool.indexOf(s) : String(s.idx), text: jobName(s.job) + ' · ' + s.w * s.d + ' soldiers' + (S.importPool.includes(s) ? ' (' + S.importName + ')' : '') }));
    sel.onchange = () => { if (!sel.value) return; pushUndo(); const s = sel.value[0] === 'i' ? S.importPool[+sel.value.slice(1)] : S.stg.troops[+sel.value]; copyTroopKind(t, s); buildWaves(); needDraw = true; toast(`Troop #${t.id} is now ${jobName(t.job)}.`); };
    const name = el('span', { class: 'wv-cell', title: t.name || '' }, el('span', { class: 'cd', style: `width:8px;height:8px;border-radius:50%;flex:none;background:${jobCat(t.job)[1]}` }), el('span', { text: tLabel(t) }));
    list.append(el('div', { class: 'wv-row wv-row2' }, name, el('span', { text: String(t.w * t.d) }), el('span', { class: 'note', text: inWaves(t.id) || 'not in any group' }), sel));
  }
  tk.append(list);
  box.append(tk);
}
function initWaves() {
  $('bWaves').onclick = () => { const p = $('waves'); p.hidden = !p.hidden; document.querySelector('.app').classList.toggle('wv-open', !p.hidden); $('bWaves').setAttribute('aria-pressed', p.hidden ? 'false' : 'true'); $('bWaves').textContent = p.hidden ? 'Waves' : 'Back to the map'; if (!p.hidden) buildWaves(); needDraw = true; };
  $('bAI').onclick = () => $('aiIn').click();
  $('aiIn').onchange = async e => {
    const f = e.target.files[0]; e.target.value = ''; if (!f) return;
    let txt; try { txt = new TextDecoder('euc-kr').decode(await f.arrayBuffer()); } catch (_) { txt = await f.text(); }
    S.ai = parseAI(txt); const n = Object.keys(S.ai).length;
    toast(n ? `Read ${n} AI order lists from ${f.name}.` : f.name + ' has no TROOP_AI_DEF lists.'); buildWaves();
  };
  $('bImp').onclick = () => $('impIn').click();
  $('impIn').onchange = async e => {
    const f = e.target.files[0]; e.target.value = ''; if (!f) return;
    try { const s = parseStg(await f.arrayBuffer()); S.importPool = s.troops.filter(t => t.job >= 0 && t.unit >= 0); S.importName = f.name; toast(`Borrowed the troop types of ${f.name}: pick them under + troop → Make a new troop like…, or under Turn it into….`, 8000); buildWaves(); }
    catch (err) { toast(err.message, 7000); }
  };
}

/* ---------- start ---------- */
buildRail(); buildLayers(); fillSelects(); initWaves();
S.m = exampleMap(); S.example = true; S.tplName = 'example (made up)';
computeTileColours();
$('banner').innerHTML = 'This is a made-up example map for trying the tools. <b>Open map files…</b> to load a real one from your game folder (<span style="font-family:var(--mono)">Data\\Mission\\Live</span>: the .map, .stg and .dds), or drop them here. New here? Press <b>Help</b>.';
$('banner').hidden = false;
setTool('select'); refreshInfo(); buildTroopList(); updUndo(); showNameHint();
/* help: shown by the Help button, and once when the editor first opens */
$('bHelp').onclick = () => { try { $('helpNoShow').checked = localStorage.getItem('bmf.helpSeen') === '1'; } catch (_) {} $('dlgHelp').showModal(); };
$('dlgHelp').addEventListener('close', () => { try { localStorage.setItem('bmf.helpSeen', $('helpNoShow').checked ? '1' : '0'); } catch (_) {} });
try { if (localStorage.getItem('bmf.helpSeen') !== '1') setTimeout(() => $('bHelp').click(), 400); } catch (_) { }
requestAnimationFrame(() => { fitView(); redrawAll(); $('hudZoom').textContent = Math.round(S.zoom * 100 / 3) + '%'; frame(); });
