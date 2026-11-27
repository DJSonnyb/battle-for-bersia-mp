const { chromium } = require('playwright'); const fs = require('fs');
const L = '/home/claude/game/Data/Mission/Live/';
(async () => {
  const b = await chromium.launch(); const pg = await b.newPage({ viewport: { width: 1400, height: 900 }, acceptDownloads: true });
  const errs = []; pg.on('pageerror', e => errs.push(e.message));
  await pg.goto('file://' + process.cwd() + '/Bersia_Map_Forge.html'); await pg.waitForTimeout(400); await pg.evaluate(() => { const d = document.getElementById('dlgHelp'); if (d && d.open) d.close(); });
  await pg.setInputFiles('#fileIn', [L + '8_Ruin.map', L + '8_Ruin.stg', L + '8_Ruin.dds']); await pg.waitForTimeout(500);
  // select area 0 via list, delete with button, add a new one
  const before = await pg.evaluate(() => [S.stg.rects.length, waveModel(S.stg).param.params.slice(0, 6).map(pInt)]);
  await pg.click('#troopList .item:has-text("Area ")');
  await pg.click('text=Delete this area');
  const mid = await pg.evaluate(() => [S.stg.rects.length, waveModel(S.stg).param.params.slice(0, 6).map(pInt)]);
  await pg.click('text=Add an area');
  const after = await pg.evaluate(() => [S.stg.rects.length, S.stg.rects[S.stg.rects.length - 1].id]);
  // props: wall list
  await pg.click('[data-tool="prop"]'); await pg.waitForTimeout(100);
  const walls = await pg.$$eval('#toolOpts .list .item .t', xs => xs.slice(0, 8).map(x => x.textContent));
  await pg.fill('#o_pfind', 'esse'); await pg.waitForTimeout(100);
  const esse = await pg.$$eval('#toolOpts .list .item .t', xs => xs.map(x => x.textContent));
  await pg.screenshot({ path: 'out/30_props.png' });
  let [dl] = await Promise.all([pg.waitForEvent('download'), pg.click('#bExport')]); await dl.saveAs('out/ruin_edit.zip');
  console.log(JSON.stringify({ errs, before, mid, after, walls, esse }));
  await b.close();
})();
