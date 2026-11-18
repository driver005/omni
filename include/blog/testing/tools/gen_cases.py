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
    if m in ('POST','PUT') and path.startswith('/members') and '/subscriptions' not in path: return 'members'
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
_KEY_FIX = {
    "/authentication/password_reset": "authentication",
    "/automated_emails/design": "automated_emails",
    "/emails/1/status": "emails",
    "/users/me/token": "users",
    "/links": "links",
    "/settings": "settings",
}
for c in (('GET', '/members/events?filter=type:comment_event,type:click_event', 'events', ''),('GET', '/members/events?filter=type:comment_event', 'events', ''),('GET', '/members/events?filter=type:click_event', 'events', ''),('GET', '/members/events?filter=type:feedback_event', 'events', ''),('GET', '/members/events?filter=type:aggregated_click_event', 'events', ''),('GET', '/members/events?filter=type:signup_event', 'events', ''),('GET', '/members/events?filter=type:email_sent_event', 'events', ''),('GET', '/members/events?filter=type:email_delivered_event', 'events', ''),('GET', '/members/events?filter=type:email_opened_event', 'events', ''),('GET', '/members/events?filter=data.post_id:', 'events', ''),('PUT', '/authentication/password_reset', 'password_reset', '{}'),('GET', '/automated_emails/design', 'automated_email_design', ''),('PUT', '/automated_emails/design', 'automated_email_design', '{}'),('POST', '/db/backup?filename=test', 'db', '{}'),('GET', '/config', 'config', ''),('GET', '/custom_theme_settings', 'custom_theme_settings', ''),('PUT', '/custom_theme_settings', 'custom_theme_settings', '{}'),('GET', '/email_previews/posts/1', 'email_previews', ''),('GET', '/email_previews/posts/1/?newsletter=1', 'email_previews', ''),('GET', '/emails', 'emails', ''),('GET', '/emails/1/status', 'email_statuses', ''),('GET', '/labels', 'labels', ''),('POST', '/labels', 'labels', '{}'),('GET', '/labels/?include=count.members', 'labels', ''),('GET', '/labels/slug/test', 'labels', ''),('GET', '/links', 'links', ''),('GET', '/links', 'bulk', ''),('PUT', '/links/bulk/?filter=1', 'links', '{}'),('PUT', '/links/bulk/?filter=1', 'bulk', '{}'),('POST', '/members/1/commenting/disable', 'email', '{}'),('POST', '/members/1/subscriptions', 'members', '{}'),('PUT', '/members/1/subscriptions/1', 'members', '{}'),('GET', '/members/?filter=newsletters.status:active', 'members', ''),('GET', '/members/?filter=newsletters.status:-active', 'members', ''),('GET', '/members/?filter=subscribed:true', 'members', ''),('GET', '/members/?filter=subscribed:false', 'members', ''),('GET', '/members/?filter=name:~', 'members', ''),('GET', '/members/?filter=status:paid&include=emailRecipients', 'members', ''),('GET', '/members/?order=email_open_rate%20desc', 'members', ''),('GET', '/members/?search=egg', 'members', ''),('GET', '/members/?search=MEMBER2', 'members', ''),('GET', '/members/?search=egon&paid=true', 'members', ''),('GET', '/members/?search=do_not_exist', 'members', ''),('GET', '/members/1', 'members', ''),('GET', '/members/1/?include=email_recipients', 'members', ''),('GET', '/members/1/?include=tiers', 'members', ''),('POST', '/members', 'email', '{}'),('PUT', '/settings', 'members', '{}'),('POST', '/members/?send_email=true&email_type=signup', 'email', '{}'),('PUT', '/members/bulk/?filter=id:', 'email', '{}'),('PUT', '/members/bulk/?all=true', 'email', '{}'),('PUT', '/members/bulk/?filter=subscribed:false', 'email', '{}'),('PUT', '/members/bulk/?filter=subscribed:true', 'email', '{}'),('GET', '/mentions/?limit=2', 'mentions', ''),('GET', '/newsletters', 'newsletters', ''),('GET', '/newsletters/?include=count.members,count.active_members,count.posts', 'newsletters', ''),('POST', '/newsletters', 'newsletters', '{}'),('POST', '/newsletters/?include=count.members,count.active_members,count.posts', 'newsletters', '{}'),('POST', '/newsletters/?opt_in_existing=true', 'newsletters', '{}'),('PUT', '/newsletters/1', 'newsletters', '{}'),('PUT', '/newsletters/1/?include=count.members,count.active_members,count.posts', 'newsletters', '{}'),('GET', '/newsletters/1', 'newsletters', ''),('GET', '/offers', 'offers', ''),('POST', '/offers', 'offers', '{}'),('GET', '/offers/1', 'offers', ''),('PUT', '/offers/1', 'offers', '{}'),('GET', '/offers/?filter=1', 'offers', ''),('PUT', '/pages/bulk/?filter=', 'bulk', '{}'),('DELETE', '/pages/?filter=', 'bulk', '{}'),('GET', '/pages/?filter=authors.password:abcd&limit=1', 'pages', ''),('POST', '/pages/?source=html&formats=mobiledoc,lexical,html', 'pages', '{}'),('POST', '/pages/?formats=mobiledoc,lexical,html', 'pages', '{}'),('PUT', '/pages/1/?formats=mobiledoc,lexical,html', 'pages', '{}'),('POST', '/pages', 'pages', '{}'),('PUT', '/pages/1', 'pages', '{}'),('PUT', '/pages/1/?formats=mobiledoc,lexical,html&convert_to_lexical=true', 'pages', '{}'),('PUT', '/posts/bulk/?filter=', 'bulk', '{}'),('DELETE', '/posts/?filter=', 'bulk', '{}'),('POST', '/posts', 'bulk', '{}'),('GET', '/posts/?limit=2', 'posts', ''),('GET', '/posts/?formats=mobiledoc,lexical,html,plaintext&limit=2', 'posts', ''),('GET', '/posts/?filter=authors.password:abcd&limit=2', 'posts', ''),('POST', '/posts/?formats=mobiledoc,lexical,html', 'posts', '{}'),('POST', '/posts', 'posts', '{}'),('PUT', '/posts/1', 'posts', '{}'),('POST', '/posts/1/copy?formats=mobiledoc,lexical', 'posts', '{}'),('PUT', '/posts/1/?formats=mobiledoc,lexical,html&convert_to_lexical=true', 'posts', '{}'),('POST', '/posts/?formats=lexical', 'posts', '{}'),('PUT', '/posts/1/?formats=lexical&save_revision=true', 'posts', '{}'),('GET', '/posts/1/?formats=lexical', 'posts', ''),('GET', '/recommendations', 'recommendations', ''),('GET', '/recommendations/?page=1&limit=10', 'recommendations', ''),('GET', '/recommendations/?include=count.clicks,count.subscribers', 'recommendations', ''),('GET', '/recommendations/?include=count.clicks', 'recommendations', ''),('GET', '/recommendations/?include=count.subscribers', 'recommendations', ''),('GET', '/recommendations/?include=count.clicks,count.subscribers&order=', 'recommendations', ''),('GET', '/recommendations/?order=', 'recommendations', ''),('GET', '/recommendations/1', 'recommendations', ''),('PUT', '/recommendations/1', 'recommendations', '{}'),('POST', '/recommendations', 'recommendations', '{}'),('POST', '/recommendations/check', 'recommendations', '{}'),('GET', '/roles', 'roles', ''),('GET', '/roles/?permissions=assign', 'roles', ''),('GET', '/search-index/posts', 'posts', ''),('GET', '/search-index/pages', 'pages', ''),('GET', '/search-index/tags', 'tags', ''),('GET', '/search-index/users', 'users', ''),('GET', '/settings', 'settings', ''),('GET', '/settings/?group=theme', 'settings', ''),('GET', '/settings/?group=core', 'settings', ''),('PUT', '/settings', 'settings', '{}'),('PUT', '/settings/verifications', 'settings', '{}'),('GET', '/site', 'site', ''),('GET', '/slugs/post/a post title', 'slugs', ''),('GET', '/slugs/post/integrations', 'slugs', ''),('GET', '/slugs/post/integrations/6194d3ce51e2700162531a71', 'slugs', ''),('GET', '/slugs/post/integrations/000000000000000000000000', 'slugs', ''),('POST', '/snippets', 'snippets', '{}'),('GET', '/snippets', 'snippets', ''),('POST', '/snippets/?formats=lexical', 'snippets', '{}'),('GET', '/snippets?formats=lexical&filter=lexical:-null', 'snippets', ''),('GET', '/stats/member_count', 'stats', ''),('GET', '/stats/mrr', 'stats', ''),('GET', '/stats/subscriptions', 'stats', ''),('GET', '/stats/referrers', 'stats', ''),('POST', '/stats/posts-visitor-counts', 'stats', '{}'),('POST', '/stats/posts-member-counts', 'stats', '{}'),('GET', '/tiers', 'tiers', ''),('GET', '/tiers/?filter=type:paid&limit=1', 'tiers', ''),('POST', '/tiers', 'tiers', '{}'),('GET', '/users/?order=id%20DESC', 'users', ''),('GET', '/users/?include=roles&order=id%20DESC', 'users', ''),('GET', '/users/?page=2&limit=5&order=id%20DESC', 'users', ''),('GET', '/users/1/?include=roles,roles.permissions,count.posts', 'users', ''),('GET', '/users/slug/1', 'users', ''),('GET', '/users/email/1', 'users', ''),('PUT', '/users/me', 'users', '{}'),('GET', '/users/1/?include=roles', 'users', ''),('PUT', '/users/me/?include=roles', 'users', '{}'),('PUT', '/users/1/?include=roles', 'users', '{}'),('PUT', '/users/owner', 'users', '{}'),('PUT', '/users/password', 'password', '{}'),('GET', '/users/me/token', 'apiKey', ''),('POST', '/webhooks', 'webhooks', '{}'),('PUT', '/webhooks/1', 'webhooks', '{}'),('GET', '/automated_emails', 'automated_emails', ''),('GET', '/automated_emails/1', 'automated_emails', ''),('POST', '/automated_emails', 'automated_emails', '{}'),('PUT', '/automated_emails/1', 'automated_emails', '{}'),('POST', '/automated_emails/1/preview', 'automated_emails', '{}'),('GET', '/automations', 'automations', ''),('GET', '/automations/1', 'automations', '')):
    c = (c[0], c[1], _KEY_FIX.get(c[1], c[2]), c[3])
    add(*c)

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
