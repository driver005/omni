import pathlib, re, json, collections

ADMIN=pathlib.Path('test/e2e-api/admin')

def blocks(src):
    """Yield (describe_chain, it_title, it_body) for each it(...) in a JS/TS test."""
    stack=[]
    i=0
    n=len(src)
    while i<n:
        m=re.search(r'\b(describe|it)\s*\(', src[i:])
        if not m: break
        pos=i+m.start()
        kind=m.group(1)
        # find opening paren of argument list, then first string literal
        p=src.find('(',pos)
        q=p+1
        while q<n and src[q] in ' \n\t': q+=1
        if q>=n or src[q] not in '`\'"':
            i=p+1; continue
        quote=src[q]; e=q+1
        while e<n and src[e]!=quote:
            if src[e]=='\\': e+=1
            e+=1
        title=src[q+1:e]
        # find matching close paren by scanning with depth
        depth=0; in_str=None; esc=False; j=p
        while j<n:
            c=src[j]
            if in_str:
                if esc: esc=False
                elif c=='\\': esc=True
                elif c==in_str: in_str=None
                j+=1; continue
            if c in '`\'"': in_str=c; j+=1; continue
            if c=='(': depth+=1
            elif c==')':
                depth-=1
                if depth==0: break
            j+=1
        body=src[p:j+1]
        if kind=='describe':
            stack.append(title)
            i=p+1  # descend into describe body to find nested it() blocks
        else:
            yield tuple(stack), title, body
            i=j+1

def requests(body):
    out=[]
    snap=body.find('matchBodySnapshot')
    for m in re.finditer(r'agent\s*\.\s*(get|post|put|delete|patch)\s*\(\s*[`\'"]([^`\'"]+)', body):
        if snap >= 0 and m.start() > snap: continue
        out.append((m.group(1).upper(), m.group(2)))
    return out if out else []

def snap_names(file):
    txt=(ADMIN/'__snapshots__'/file.replace('.test.js','.test.js.snap').replace('.test.ts','.test.ts.snap')).read_text()
    return set(re.findall(r'exports\[`([^`]+)`\]',txt))

cases=[]
for f in sorted(ADMIN.glob('*.test.js'))+sorted(ADMIN.glob('*.test.ts')):
    snap=ADMIN/'__snapshots__'/(f.name.replace('.test.js','.test.js.snap').replace('.test.ts','.test.ts.snap'))
    if not snap.exists(): continue
    names=snap_names(f.name)
    src=f.read_text()
    for chain,title,body in blocks(src):
        reqs=requests(body)
        if not reqs: continue
        # match snapshot names ending with this it title + ' 1: [body] 1'
        nt = title.split('${')[0].strip()
        matched=[n for n in names if ('[body]' in n) and
                 ((title in n) or (len(nt) > 8 and nt in n))]
        if not matched: continue
        for method,path in reqs:
            path=re.sub(r'\$\{[^}]+\}','1',path)
            if not path.startswith('/'): path='/'+path
            path=path.rstrip('/') or '/'
            for n in matched:
                cases.append((f.name,method,path,n))
for c in cases[:15]: print(c[1],c[2],'<=',c[3][:70])
json.dump(cases,open('/tmp/opencode/snapshot_map.json','w'),indent=1)
