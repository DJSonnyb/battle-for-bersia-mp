const { chromium } = require('playwright'); const fs = require('fs'); const C = require('./core.js');
const L = '/home/claude/game/Data/Mission/Live/';
(async () => {
  const b = await chromium.launch(); const pg = await b.newPage({ viewport: { width: 1400, height: 900 }, acceptDownloads: true });
  const errs = []; pg.on('pageerror', e => errs.push(e.message));
  await pg.goto('file://' + process.cwd() + '/test.html'); await pg.waitForTimeout(400); await pg.evaluate(() => { const d = document.getElementById('dlgHelp'); if (d && d.open) d.close(); });
  await pg.setInputFiles('#fileIn', [L + '4_Colonock.map', L + '4_Colonock.stg', L + '4_Colonock.dds']); await pg.waitForTimeout(600);
  await pg.selectOption('#nPlayers', '6'); await pg.waitForTimeout(300);
  await pg.screenshot({ path: 'out/20_players6.png' });
  const name = await pg.inputValue('#expName');
  let [dl] = await Promise.all([pg.waitForEvent('download'), pg.click('#bExport')]); await dl.saveAs('out/p6.zip');
  await pg.selectOption('#nPlayers', '2');
  [dl] = await Promise.all([pg.waitForEvent('download'), pg.click('#bExport')]); await dl.saveAs('out/p2.zip');
  await pg.keyboard.press('Control+z'); const und = await pg.evaluate(() => S.stg.troops.length);
  console.log(JSON.stringify({ errs, name, und }));
  await b.close();
})();
