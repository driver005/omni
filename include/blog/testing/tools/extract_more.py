import importlib.util, collections, json, pathlib
import pathlib
_HERE=pathlib.Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('sn',_HERE/'snapshot_normalize.py')
sn=importlib.util.module_from_spec(spec); spec.loader.exec_module(sn)
SNAP=sn.SNAP
def collect(file, key):
    out={'top':set(),'fields':collections.defaultdict(set),'meta':set(),'nested':collections.defaultdict(set)}
    for name,obj in sn.bodies(file):
        for k,v in obj.items():
            out['top'].add(k)
            if k==key and v[0]=='arr':
                for el in v[1]:
                    if el[0]=='obj':
                        for fk,fv in el[1].items():
                            out['fields'].setdefault(fk,set()).add(sn.type_of(fv))
                            if fv[0]=='obj':
                                for sk,sv in fv[1].items(): out['nested'].setdefault(f'{fk}.{sk}',set()).add(sn.type_of(sv))
            if k=='meta' and v[0]=='obj':
                for sk,sv in v[1].items(): out['meta'].add(sk)
    return out
res={}
for key,fn in [('users','users.test.js.snap'),('newsletters','newsletters.test.js.snap'),('tiers','tiers.test.js.snap')]:
    r=collect(SNAP/fn,key); res[key]=r
    print(f'== {key} top={sorted(r["top"])} meta={sorted(r["meta"])}')
    print(f'   fields ({len(r["fields"])}): {sorted(r["fields"])}')
pathlib.Path('/tmp/opencode/ghost_schema_more.json').write_text(json.dumps({k:{
  'top':sorted(v['top']),'meta':sorted(v['meta']),
  'fields':{f:sorted(t) for f,t in v['fields'].items()},
  'nested':{f:sorted(t) for f,t in v['nested'].items()}} for k,v in res.items()},indent=2))
print('wrote')
