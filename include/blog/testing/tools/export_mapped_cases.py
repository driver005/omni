import importlib.util, collections, json, re
spec=importlib.util.spec_from_file_location('mp','include/blog/testing/tools/map_snapshots.py')
mp=importlib.util.module_from_spec(spec); spec.loader.exec_module(mp)
sn_spec=importlib.util.spec_from_file_location('sn','include/blog/testing/tools/snapshot_normalize.py')
sn=importlib.util.module_from_spec(sn_spec); sn_spec.loader.exec_module(sn)
cases=json.load(open('/tmp/opencode/snapshot_map.json'))
bodies_cache={}
out=[]
for f,method,path,name in cases:
    if f not in bodies_cache:
        snapf=sn.SNAP/(f.replace('.test.js','.test.js.snap').replace('.test.ts','.test.ts.snap'))
        bodies_cache[f]={n:o for n,o in sn.bodies(snapf)}
    obj=bodies_cache[f].get(name)
    if not obj: continue
    keys=[k for k in obj if k not in ('meta','errors','headers')]
    if not keys: continue
    key=keys[0]
    if key in ('bulk','data','resource','total','status','updated_at','uuid','note','subscribed','subscriptions','unsubscribe_url','latest','latestOpened','missing','scheduled'):
        key='members' if 'status' in obj else key
    path=re.sub(r'\$\{[^}]+\}','1',path)
    if not path.startswith('/'): path='/'+path
    path=path.rstrip('/') or '/'
    if method in ('POST','PUT','PATCH') and path.startswith('/members') and '/subscriptions' not in path:
        key='email'
    body='{}' if method!='GET' else ''
    out.append((method,path,key,body))
seen=set(); uniq=[]
for c in out:
    k=(c[0],c[1],c[2])
    if k in seen: continue
    seen.add(k); uniq.append(c)
print('mapped replay cases',len(uniq))
json.dump(uniq,open('/tmp/opencode/mapped_cases.json','w'),indent=1)
for c in uniq[:8]: print(c)
