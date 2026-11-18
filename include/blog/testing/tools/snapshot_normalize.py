import re, json, collections, pathlib

SNAP = pathlib.Path('test/e2e-api/admin/__snapshots__')

def strip_commas(line):
    return line.rstrip().rstrip(',')

def parse_value(text, i):
    """Parse a Jest-serialized value starting at i. Returns (node, next_i).
    node = ('obj', {k:node}) | ('arr', [node]) | ('str',None) | ('num',None)
           | ('bool',None) | ('null',None) | ('match',label)"""
    n=len(text)
    while i<n and text[i] in ' \n\t': i+=1
    if i>=n: return ('null',None), i
    c=text[i]
    if c=='O' and text.startswith('Object {',i):
        i+=len('Object {'); out={}
        guard=0
        while i<n:
            guard+=1
            if guard>100000: break
            while i<n and text[i] in ' \n\t,': i+=1
            if i<n and text[i]=='}': i+=1; break
            m=re.match(r'"((?:[^"\\]|\\.)*)"\s*:\s*',text[i:])
            if not m: break
            key=bytes(m.group(1),'utf-8').decode('unicode_escape')
            before=i
            i+=m.end()
            val,i=parse_value(text,i)
            if i<=before: i=before+1
            out[key]=val
        return ('obj',out),i
    if c=='A' and text.startswith('Array [',i):
        i+=len('Array ['); out=[]
        guard=0
        while i<n:
            guard+=1
            if guard>100000: break
            while i<n and text[i] in ' \n\t,': i+=1
            if i<n and text[i]==']': i+=1; break
            if i<n and text[i]=='#':
                j=text.find('\n',i); i=(j+1) if j!=-1 else n; continue
            if i<n and text[i]=='<':
                j=text.find('>',i); i=(j+1) if j!=-1 else n; continue
            before=i
            val,i=parse_value(text,i)
            if i<=before: i=before+1; continue
            out.append(val)
        return ('arr',out),i
    if text.startswith('Any<',i):
        j=text.find('>',i); label=text[i:j+1]; return ('match',label),j+1
    if text.startswith('Nullable<',i):
        j=text.find('>',i); label=text[i:j+1]; return ('match',label),j+1
    if text.startswith('StringMatching',i) or text.startswith('StringContaining',i) or text.startswith('StringNotContaining',i):
        # matchers have no braces; consume to end of line
        j=text.find('\n',i); return ('match',text[i:j].strip()[:30]),(j+1 if j!=-1 else n)
    if c=='"':
        j=re.match(r'"((?:[^"\\]|\\.)*)"',text[i:]);
        if j: return ('str',j.group(1)),i+j.end()
        return ('str',None),i+1
    if c=='t' and text.startswith('true',i): return ('bool',None),i+4
    if c=='f' and text.startswith('false',i): return ('bool',None),i+5
    if c=='n' and text.startswith('null',i): return ('null',None),i+4
    if c in '-0123456789':
        m=re.match(r'-?\d+(?:\.\d+)?',text[i:])
        if m: return ('num',None),i+m.end()
        return ('num',None),i+1
    # unknown bare token -> consume to delimiter
    j=min([x for x in (text.find(',',i),text.find('\n',i),text.find('}',i)) if x!=-1] or [n])
    return ('match',text[i:j].strip()[:30]),j

def bodies(file):
    txt=file.read_text()
    for m in re.finditer(r'exports\[`([^`]*\[body\][^`]*)`\]\s*=\s*`([\s\S]*?)`;',txt):
        name, val = m.group(1), m.group(2)
        try:
            node,_=parse_value(val,0)
        except Exception:
            continue
        if node[0]=='obj': yield name,node[1]

def type_of(node):
    k=node[0]
    if k=='match': return 'string' if 'String' in (node[1] or '') else 'any'
    return {'obj':'object','arr':'array','str':'string','num':'number','bool':'boolean','null':'null'}[k]

def collect(file, entity_key, out):
    for name,obj in bodies(file):
        for k,v in obj.items():
            out['top'].add(k)
            if k==entity_key and v[0]=='arr':
                for el in v[1]:
                    if el[0]=='obj':
                        for fk,fv in el[1].items():
                            out['fields'].setdefault(fk,set()).add(type_of(fv))
                            # nested object subfields
                            if fv[0]=='obj':
                                for sk,sv in fv[1].items():
                                    out['nested'].setdefault(f'{fk}.{sk}',set()).add(type_of(sv))
            if k=='meta' and v[0]=='obj':
                for sk,sv in v[1].items(): out['meta'].add(sk)

posts=collections.defaultdict(lambda:{'top':set(),'fields':collections.defaultdict(set),'nested':collections.defaultdict(set),'meta':set()})
members=collections.defaultdict(lambda:{'top':set(),'fields':collections.defaultdict(set),'nested':collections.defaultdict(set),'meta':set()})
collect(SNAP/'posts.test.js.snap','posts',posts['posts'])
collect(SNAP/'pages.test.js.snap','pages',pages_dummy if False else posts['pages'])
collect(SNAP/'members.test.js.snap','members',members['members'])

def show(label,res):
    print(f'=== {label} top: {sorted(res["top"])}')
    print(f'    meta: {sorted(res["meta"])}')
    fs=sorted(res['fields'])
    print(f'    fields ({len(fs)}): {fs}')
    print(f'    nested ({len(res["nested"])}): {sorted(res["nested"])[:40]}')

show('POSTS',posts['posts'])
show('PAGES',posts['pages'])
show('MEMBERS',members['members'])

# dump structured schema to json for C++ generation
schema={}
for label,res in [('posts',posts['posts']),('pages',posts['pages']),('members',members['members'])]:
    schema[label]={
        'top':sorted(res['top']),
        'meta':sorted(res['meta']),
        'fields':{k:sorted(v) for k,v in res['fields'].items()},
        'nested':{k:sorted(v) for k,v in res['nested'].items()},
    }
pathlib.Path('/tmp/opencode/ghost_schema.json').write_text(json.dumps(schema,indent=2))
print('wrote /tmp/opencode/ghost_schema.json')
