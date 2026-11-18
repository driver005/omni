import re, json
routes=json.load(open('/tmp/opencode/ghost_routes.json'))

def key_for(res, path):
    special = {
        'config':'config', 'site':'title', 'oembed':'iframe', 'stats':'stats',
        'feedback':'message', 'featurebase':'config',
        'db':'db', 'email_previews':'email_previews',
        'identities':'identity', 'session':'session', 'exports':'posts',
        'slack':'ok', 'tinybird':'tinybird', 'themes':'themes', 'upgrades':'upgrades',
        'redirects':'redirects', 'schedules':'schedules', 'slugs':'slugs',
        'media':'media', 'files':'files', 'images':'images', 'mentions':'mentions',
        'exports':'exports', 'slack':'slack', 'featurebase':'featurebase',
        'identities':'identities',
    }
    if path.startswith('/stripe/checkout/preview'): return 'preview'
    if path.startswith('/stripe/checkout/branding'): return 'branding'
    if path.startswith('/stripe/checkout/config'): return 'config'
    if res in special: return special[res]
    if m in ('POST','PUT') and path.startswith('/members') and '/subscriptions' not in path: return 'email'
    if path == '/members/events': return 'events'
    if path.endswith('/likes'): return 'comment_likes'
    if path.endswith('/dislikes'): return 'comment_dislikes'
    if path.endswith('/reports'): return 'comment_reports'
    if path.startswith('/search-index/'): return path.rsplit('/',1)[1]
    if '/gift_links' in path: return 'gift_links'
    if path.startswith('/stripe/checkout/preview'): return 'preview'
    if path.startswith('/stripe/checkout/branding'): return 'branding'
    if path.endswith('/signin_urls'): return 'signin_urls'
    if path.endswith('/links'): return 'links'
    if path.endswith('/performance-stats'): return 'stats'
    if path.endswith('/email_preview'): return 'automation_email_previews'
    if path.endswith('/batches'): return 'batches'
    if path.endswith('/recipient-failures'): return 'failures'
    if path.startswith('/emails/') and 'analytics' in path: return 'latest'
    if path.startswith('/members/stats'): return 'stats'
    if path=='/users/password': return 'password'
    return res

def body_for(method, path):
    if method=='GET': return ''
    if path=='/posts': return '{"title":"Replay","slug":"replay","status":"draft"}'
    if path.startswith('/automations/') and path.endswith('/email_preview'):
        return '{"subject":"Replay","lexical":"{}"}'
    return '{}'

cases=[]
seen=set()
def add(m,p,k,b):
    key=(m,p,k)
    if key in seen: return
    seen.add(key); cases.append((m,p,k,b))

# 1) every route, concrete params -> seeded id 1
for api,m,p0,res,act in routes:
    m='GET' if m=='ALL' else m
    p=re.sub(r':\w+','1',p0.rstrip('/') or '/')
    add(m,p,key_for(res,p),body_for(m,p))

# 2) query variants on the main collection GETs
collections=sorted({p0.rstrip('/').lstrip('/') for api,m,p0,res,act in routes
                    if m in ('GET','ALL') and ':' not in p0 and '/' not in p0.rstrip('/').lstrip('/')})
variants=['fields=id,name','fields=title,slug','fields=id','fields=name,slug',
          'filter=status:draft','filter=status:published','filter=status:free',
          'filter=status:paid','include=authors','include=tags','include=newsletters',
          'limit=1','limit=2','limit=5','page=1','page=2','offset=1',
          'order=title DESC','order=created_at ASC','order=name ASC',
          'fields=id','filter=status:active','include=post','include=members',
          'limit=10','limit=25','page=3','offset=5','order=id ASC','order=updated_at DESC']
res_of={}
for api,m,p0,res,act in routes:
    bare=p0.rstrip('/').lstrip('/')
    if m in ('GET','ALL') and ':' not in p0 and '/' not in bare:
        res_of.setdefault(bare,res)
for c in collections:
    vkey=key_for(res_of.get(c,c), '/'+c)
    for v in variants:
        add('GET', '/'+c+'?'+v, vkey, '')

# 3) hand-picked value/shape assertions (from prior table)
extra=[
 ('GET','/posts/1','posts',''),('GET','/members/1/signin_urls','signin_urls',''),
 ('GET','/tags/slug/getting-started','tags',''),('GET','/users/slug/ghost-author','users',''),
 ('GET','/automations/1/runs','automations',''),('GET','/automations/1/performance-stats','stats',''),
 ('POST','/automations/1/email_test','errors','{"email":"bad"}'),
 ('POST','/posts','errors','{"slug":"no-title"}'),
 ('GET','/labels','labels',''),('GET','/stats/subscriptions','stats',''),
 ('GET','/automations','automations',''), ('GET','/members/events','events',''),('GET','/members/events?filter=type:comment_event','events',''),
 ('GET','/posts','Ghostly Kitchen Sink',''),('GET','/posts','ghostly-kitchen-sink',''),
 ('GET','/members','member2@test.com',''),('GET','/settings','Thoughts, stories and ideas',''),
 ('GET','/posts?filter=status:published','ghostly-kitchen-sink',''),
 ('GET','/members?filter=status:paid','members',''),('GET','/offers?fields=name','offers',''),
 ('GET','/members','meta',''),('GET','/newsletters','meta',''),('GET','/settings','meta',''),
 ('GET','/tiers','meta',''),
 ('POST','/members/1/subscriptions/','members','{"tier":"gold"}'),
 ('PUT','/members/1/subscriptions/1','members','{"tier":"gold","status":"canceled"}'),
 ('GET','/config/featurebase','testing-mysql',''),('GET','/config/featurebase','mysql8',''),
 ('GET','/site','#FF1A75',''),('GET','/site','Etc/UTC',''),('GET','/site','Thoughts, stories and ideas',''),
 ('GET','/tags','News',''),('GET','/users','Ghost Author',''),('GET','/tiers','Free',''),
 ('GET','/email_previews/posts/1','email_previews',''),
 ('GET','/comments/1/replies','comments',''),('GET','/comments/1/likes','comment_likes',''),
 ('GET','/comments/1/reports','comment_reports',''),('GET','/comments/1/dislikes','comment_dislikes',''),
 ('GET','/search-index/posts','posts',''),('GET','/search-index/pages','pages',''),
]
for c in extra: add(*c)

def q(v): return '"'+v.replace('\\','\\\\').replace('"','\\"')+'"'
entries='\n'.join(f'        SnapshotCase{{"{m}","{p}","{k}",{q(b)}}}'+(',' if i+1<len(cases) else '') for i,(m,p,k,b) in enumerate(cases))
content=f'''export module blog_testing:snapshot_cases;

import std;

export namespace blog::testing {{

struct SnapshotCase
{{
    std::string_view method;
    std::string_view path;
    std::string_view key;
    std::string_view body;
}};

struct SnapshotCases
{{
    static constexpr std::array<SnapshotCase, {len(cases)}> values{{
{entries}
    }};
}};

}} // namespace blog::testing
'''
open('include/blog/testing/snapshot_cases.cppm','w').write(content)
print('cases',len(cases))
