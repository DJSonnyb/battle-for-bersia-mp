p=open('page.html').read(); s=open('app.js').read()
p=p.replace('%%CORE%%',open('core.js').read()).replace('%%CAT%%',open('propcat.min.json').read().strip()).replace('%%APP%%',s)
open('bersia-map-forge.html','w').write(p)
open('test.html','w').write('<!doctype html><html><head><meta charset=utf8><meta name=viewport content="width=device-width,initial-scale=1"></head><body>'+p+'</body></html>')

open('Bersia_Map_Forge.html','w').write('<!doctype html>\n<html lang="en">\n<head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover"><style>body{margin:0;font:14px system-ui,sans-serif}img{max-width:100%}[hidden]{display:none!important}</style></head>\n<body>\n'+p+'\n</body>\n</html>\n')
