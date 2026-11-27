const { chromium } = require('playwright'); const fs = require('fs'); const C = require('./core.js');
const L = '/home/claude/game/Data/Mission/Live/';
(async () => {
  const b = await chromium.launch(); const pg = await b.newPage({ viewport: { width: 1400, height: 860 }, acceptDownloads: true });
  const errs = []; pg.on('pageerror', e => errs.push(e.message)); pg.on('console', m => { if (m.type() === 'error') errs.push(m.text()); });
  await pg.goto('file://' + process.cwd() + '/test.html'); await pg.waitForTimeout(800); await pg.evaluate(() => { const d = document.getElementById('dlgHelp'); if (d && d.open) d.close(); });
  await pg.screenshot({ path: 'out/01_example.png' });
  await pg.setInputFiles('#fileIn', [L + '9_IceStorm.map', L + '9_IceStorm.stg', L + '9_IceStorm.dds']); await pg.waitForTimeout(800);
  await pg.screenshot({ path: 'out/02_icestorm.png' });
  // export unchanged
  let [dl] = await Promise.all([pg.waitForEvent('download'), pg.click('#bExport')]); await dl.saveAs('out/unchanged.zip');
  // edits: raise hill, plant forest, block, move a troop and an area
  const box = await pg.locator('#view').boundingBox(); const cx = box.x + box.width / 2, cy = box.y + box.height / 2;
  await pg.click('[data-tool="raise"]'); await pg.mouse.move(cx, cy); await pg.mouse.down(); for (let i = 0; i < 20; i++) await pg.mouse.move(cx + i * 3, cy + i); await pg.mouse.up();
  await pg.click('[data-tool="forest"]'); await pg.mouse.move(cx - 150, cy - 100); await pg.mouse.down(); for (let i = 0; i < 20; i++) await pg.mouse.move(cx - 150 + i * 4, cy - 100); await pg.mouse.up();
  await pg.click('[data-tool="block"]'); await pg.mouse.move(cx + 100, cy + 150); await pg.mouse.down(); await pg.mouse.move(cx + 200, cy + 150); await pg.mouse.up();
  await pg.click('[data-tool="paint"]'); await pg.mouse.move(cx - 50, cy + 150); await pg.mouse.down(); await pg.mouse.move(cx + 30, cy + 170); await pg.mouse.up();
  await pg.click('[data-tool="select"]');
  const tp = await pg.evaluate(() => { const t = S.stg.troops[0]; const [cx, cy] = worldToCell(t.x, t.y); const [sx, sy] = toScreen(cx, cy); const r = view.getBoundingClientRect(); return [r.left + sx, r.top + sy, t.x, t.y]; });
  await pg.mouse.move(tp[0], tp[1]); await pg.mouse.down(); await pg.mouse.move(tp[0] + 60, tp[1] - 40, { steps: 5 }); await pg.mouse.up();
  await pg.screenshot({ path: 'out/03_edited.png' });
  const after = await pg.evaluate(() => [S.stg.troops[0].x, S.stg.troops[0].y, S.m.props.length]);
  await pg.fill('#expName', '9_TestMap');
  [dl] = await Promise.all([pg.waitForEvent('download'), pg.click('#bExport')]); await dl.saveAs('out/edited.zip');
  [dl] = await Promise.all([pg.waitForEvent('download'), pg.click('#bProject')]); await dl.saveAs('out/proj.mapforge.json');
  // undo all then redo once
  for (let i = 0; i < 10; i++) await pg.keyboard.press('Control+z');
  const undone = await pg.evaluate(() => [S.stg.troops[0].x, S.m.props.length]);
  // reload page, open original + project, export again -> should equal edited
  await pg.reload(); await pg.waitForTimeout(500); await pg.evaluate(() => { const d = document.getElementById('dlgHelp'); if (d && d.open) d.close(); });
  await pg.setInputFiles('#fileIn', [L + '9_IceStorm.map', L + '9_IceStorm.stg', L + '9_IceStorm.dds', 'out/proj.mapforge.json']); await pg.waitForTimeout(800);
  [dl] = await Promise.all([pg.waitForEvent('download'), pg.click('#bExport')]); await dl.saveAs('out/fromproj.zip');
  // new flat map with stg kept
  await pg.click('#bNew'); await pg.click('#dlgNew button[value=ok]'); await pg.waitForTimeout(300);
  await pg.screenshot({ path: 'out/04_flat.png' });
  await pg.setViewportSize({ width: 400, height: 800 }); await pg.waitForTimeout(300); await pg.screenshot({ path: 'out/05_phone.png', fullPage: true });
  const ov = await pg.evaluate(() => document.documentElement.scrollWidth - innerWidth);
  console.log(JSON.stringify({ errs, tp: tp.slice(2), after, undone, phoneOverflow: ov }));
  await b.close();
})();
