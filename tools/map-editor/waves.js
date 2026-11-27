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
