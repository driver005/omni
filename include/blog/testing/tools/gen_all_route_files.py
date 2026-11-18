"""Regenerate every domain route file so each verb/path calls its generated
self-contained operation handler. Posts keeps its hand-tuned file."""
import json, pathlib, re
from importlib.machinery import SourceFileLoader
mod=SourceFileLoader('ops','include/blog/testing/tools/gen_all_operations.py').load_module()
ROUTES=json.load(open('/tmp/opencode/ghost_routes.json'))
OUT=pathlib.Path('include/blog/api')

def newnode(): return {'__methods__':[]}
g={}
for api,m,p,res,act in ROUTES:
    segs=[s if not s.startswith(':') else ':id' for s in (x for x in p.rstrip('/').split('/') if x)]
    if not segs: continue
    node=g.setdefault(segs[0],{'__methods__':[]})
    if len(segs)>1:
        for s in segs[1:-1]: node=node.setdefault(s,{'__methods__':[]})
        node=node.setdefault(segs[-1],{'__methods__':[]})
    if m not in node['__methods__']: node['__methods__'].append(m)

def handler_ref(m,p):
    domain=mod.domain_of(p)
    op=mod.op_name(m,p)
    cls=mod.cls(op)+mod.cls(domain)+'Handler'
    return f'blog::handlers::operations::{domain}::{op}::{cls}::handle'

def h(m,p):
    ref=handler_ref(m,p)
    return f'''[h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {{
        {ref}(req, res, std::move(send));
    }}'''

def verb(m): return {'GET':'get','POST':'post','PUT':'put','DELETE':'delt','ALL':'get','PATCH':'patch'}[m]

def emit(node, seg, ind, as_router, full_path):
    out=[f'{ind}{"Router" if as_router else "Route"}{{router, "/{seg}"}}' if as_router else f'{ind}Route{{"/{seg}"}}']
    ci=ind+'    '
    for m in node['__methods__']:
        out.append(f'{ci}.{verb(m)}({h(m, full_path)})')
        out.append(f'{ci}.summary("Ghost {seg} {m}")')
    for child,sub in sorted(node.items()):
        if child=='__methods__': continue
        cp=f'{full_path}/{child}'
        is_router=any(k!='__methods__' for k in sub)
        out.append(f'{ci}.add_{"router" if is_router else "route"}(')
        out.append(emit(sub,child,ci+'    ',is_router,cp))
        out.append(f'{ci})')
    return '\n'.join(out)

for head,node in sorted(g.items()):
    if head=='posts': continue
    name=re.sub(r'[^a-z0-9]','_',head)
    cls=''.join(p.capitalize() for p in name.split('_'))
    body=emit(node,head,'        ',True,'/'+head)
    content=f'''export module blog_api:{name}_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {{

/** @brief Ghost routes for /{head}. */
class {cls}Routes
{{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::{cls}RouteHandler& /*handler*/
    )
    {{
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
{body};
    }}
}};

}} // namespace blog::api
'''
    (OUT/f'{name}_routes.cppm').write_text(content)
print('regenerated route files')
