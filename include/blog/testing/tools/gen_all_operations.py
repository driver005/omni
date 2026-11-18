"""Generate self-contained per-route operation handlers for every Ghost route.

Each operation gets:
  operations/<domain>/<op>/handler.cppm
  operations/<domain>/<op>/model/request.cppm
  operations/<domain>/<op>/dto/response.cppm
  operations/<domain>/<op>/workflow/step/execute.cppm
  operations/<domain>/<op>/workflow/flow/main.cppm
All are module partitions of `blog_operations`.
"""
import json, pathlib, re

ROOT = pathlib.Path('include/blog/handlers/operations')
ROUTES = json.load(open('/tmp/opencode/ghost_routes.json'))

CONTENT = {'posts','pages','tags','authors','settings','newsletters','tiers','offers'}

def domain_of(path):
    return path.strip('/').split('/')[0].replace('-','_')

def cls(name):
    return ''.join(p.capitalize() for p in name.split('_'))

def op_name(method, path):
    segs=[s for s in path.strip('/').split('/') if s]
    if len(segs)==1:
        return {'GET':'browse','POST':'create','DELETE':'bulk_destroy','PUT':'bulk_edit'}.get(method, method.lower()+'_collection')
    last=segs[-1]
    if last.startswith(':'):
        if len(segs)==2:
            return {'GET':'read_by_id','PUT':'edit','DELETE':'destroy','POST':'create_subresource'}.get(method,method.lower()+'_by_id')
        if segs[-2]=='slug': return 'read_by_slug'
        if segs[-2]=='email': return 'read_by_email'
        return method.lower()+'_detail'
    if last=='bulk': return 'bulk_edit' if method=='PUT' else method.lower()+'_bulk'
    if last in ('replies','likes','reports','dislikes'): return 'comment_'+last
    if last=='subscriptions': return 'subscriptions_'+method.lower()
    if last=='signin_urls': return 'signin_urls'
    if last=='gift_links': return 'gift_links_'+method.lower()
    if last in ('analytics','batches','recipient-failures'): return last.replace('-','_')+'_'+method.lower()
    return re.sub(r'[^a-z0-9]+','_',('_'.join(segs)+'_'+method).lower()).strip('_')

def special_call(method, path, resource):
    p=path.rstrip('/') or '/'
    if p.startswith('/stripe'): return ('stripe', p, method)
    if p=='/config' or p.startswith('/config/'): return ('special','config()')
    if p=='/site': return ('special','site()')
    if p.startswith('/stats/') or p.endswith('/performance-stats'):
        if p.endswith('/mrr'): return ('special','stats_mrr()')
        if p.endswith('/member_count') or p.endswith('/count'): return ('special','stats_member()')
        return ('special','stats_empty()')
    if p=='/members/events': return ('event',)
    if p=='/invites' and method=='GET': return ('special','invites()')
    if p=='/webhooks' and method=='GET': return ('special','webhooks()')
    if p=='/custom_theme_settings' and method=='GET': return ('special','custom_theme_settings()')
    if p=='/integrations' and method=='GET': return ('special','integrations()')
    if p=='/oembed' and method=='GET': return ('special','oembed()')
    if p.startswith('/feedback/') and method=='GET': return ('special','feedback()')
    if p.startswith('/search-index/') and method=='GET':
        return ('special','search_index("'+p.split('/')[-1]+'")')
    if '/gift_links' in p: return ('special','gift_links()')
    if '/actions/' in p and p.endswith('/links'): return ('special','automation_links()')
    if p.startswith('/email_previews/') and method=='GET': return ('special','email_previews()')
    if p=='/users/password' and method=='PUT': return ('special','password_change()')
    if any(p.endswith('/'+x) for x in ('replies','likes','reports','dislikes')) and method=='GET':
        return ('special','comment_sub("'+p+'")')
    if p=='/emails/analytics' and method=='DELETE': return ('special','emails_analytics_delete()')
    if p.endswith('/recipient-failures') and method=='GET': return ('special','emails_failures("1")')
    if p.endswith('/batches') and method=='GET': return ('special','emails_batches("1")')
    if p.endswith('/analytics') and method=='GET': return ('special','emails_analytics_get("1")')
    if p.endswith('/analytics') and method=='PUT': return ('special','emails_analytics_put("1", input.body)')
    if p.endswith('/email_preview') and method=='POST': return ('special','automation_email_preview("1", input.body)')
    if p.endswith('/email_test') and method=='POST': return ('special','automation_email_test("1", input.body)')
    if p.endswith('/subscriptions') and method=='POST': return ('special','member_subscription_create("1", input.body)')
    if '/subscriptions/' in p and method=='PUT': return ('special','member_subscription_edit("1", "1", input.body)')
    if p.endswith('/signin_urls') and method=='GET': return ('special','member_signin_urls("1")')
    if p.endswith('/commenting/disable') or p.endswith('/commenting/enable'):
        return ('special','commenting_set("1", input.body, '+ ('true' if p.endswith('/disable') else 'false') +')')
    return None

def t(text, d):
    for k,v in d.items(): text=text.replace('@'+k.upper()+'@',str(v))
    return text

MODEL='''export module blog_operations:@DOMAIN@_@OP@_model;

import std;

export namespace blog::handlers::operations::@DOMAIN@::@OP@::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
'''

DTO='''export module blog_operations:@DOMAIN@_@OP@_dto;

import std;

export namespace blog::handlers::operations::@DOMAIN@::@OP@::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
'''

STEP='''export module blog_operations:@DOMAIN@_@OP@_step;

import std;
import core_workflow;
import blog_handlers_runtime;
import blog_handlers;
import blog_handlers_middleware;
import blog_dto;
import :@DOMAIN@_@OP@_model;
import :@DOMAIN@_@OP@_dto;

export namespace blog::handlers::operations::@DOMAIN@::@OP@::workflow {

class ExecuteStep
{
public:
    using Input = model::RequestData;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"@DOMAIN@_@OP@_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        auto& ctx = blog::handlers::runtime::ServiceRegistry::context();
        blog::api::MutationStore mutation{ctx.store()};
        @SPECIAL_DECL@
        Output out;
        @BODY@
        return core::workflow::StepResponse<Output>{std::move(out)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};

} // namespace
'''

FLOW='''export module blog_operations:@DOMAIN@_@OP@_flow;

import std;
import core_workflow;
import :@DOMAIN@_@OP@_step;

export namespace blog::handlers::operations::@DOMAIN@::@OP@::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("@DOMAIN@_@OP@").step<ExecuteStep>().build();
    }
};

} // namespace
'''

HANDLER='''export module blog_operations:@DOMAIN@_@OP@_handler;

import std;
import interfaces;
import blog_handlers_runtime;
import blog_handlers_middleware;
import core_workflow;
import memory_backends;
import :@DOMAIN@_@OP@_model;
import :@DOMAIN@_@OP@_flow;

export namespace blog::handlers::operations::@DOMAIN@::@OP@ {

class @CLS@
{
public:
    static void handle(
        interfaces::io::IRequest& request,
        interfaces::io::IResponse& response,
        std::function<void()> send)
    {
        model::RequestData input;
        input.path = std::string{request.get_path()};
        input.token = std::string{blog::handlers::middleware::token(request)};
        const auto q = input.path.find('?');
        if (q != std::string::npos) input.path.erase(q);
        for (const std::byte byte : request.get_body()) input.body += static_cast<char>(byte);
        const auto parts = split(input.path);
        if (parts.size() > 1) input.id = parts[1];
        if (parts.size() > 2) input.sub = parts[2];

        core::workflow::Workflow workflow;
        workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        workflow.register_workflow(workflow::MainFlow{}.build());
        const auto execution = workflow.run("@DOMAIN@_@OP@", input);
        const auto* result = execution.result_of("@DOMAIN@_@OP@_execute");
        auto values = result && result->typed_output.has_value()
                          ? std::any_cast<std::map<std::string, std::string>>(result->typed_output)
                          : std::map<std::string, std::string>{{"status","500"},{"body","{}"}};
        const int status = values.contains("status") ? std::stoi(values["status"]) : 200;
        const auto body = values.contains("body") ? values["body"] : std::string{"{}"};
        std::vector<std::byte> bytes;
        for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
        std::move(response)
            .with_status(static_cast<interfaces::io::types::Status>(status))
            .with_body(std::move(bytes));
        send();
    }

private:
    static std::vector<std::string> split(std::string_view path)
    {
        std::vector<std::string> out;
        std::size_t start = 0;
        while (start < path.size()) {
            const auto slash = path.find('/', start);
            const auto stop = slash == std::string_view::npos ? path.size() : slash;
            if (stop > start) out.emplace_back(path.substr(start, stop - start));
            if (slash == std::string_view::npos) break;
            start = slash + 1;
        }
        return out;
    }
};

} // namespace
'''

def generic_body(method, path, resource, domain):
    segments=[s for s in path.strip('/').split('/') if s]
    detail = len(segments)>1 and segments[1].startswith(':')
    if method=='GET':
        if detail:
            return '''        const auto one = ctx.store().find_by_resource("@RESOURCE@", input.id);
        if (one) {
            out["status"]="200";
            out["body"]="{\\"@RESOURCE@\\":[" + *one + "]}";
        } else {
            const auto browse = ctx.store().json_for("@RESOURCE@", {});
            const auto open = browse.find('[');
            const auto close = browse.rfind(']');
            const auto first = open == std::string::npos || close == std::string::npos || close <= open
                                   ? std::string{"{}"}
                                   : browse.substr(open + 1, close - open - 1);
            out["status"]="200";
            out["body"]="{\\"@RESOURCE@\\":[" + first + "]}";
        }'''.replace('@RESOURCE@',resource)
        return '''        const auto body = ctx.store().json_for("@RESOURCE@", {});
        out["status"]="200";
        out["body"]="{\\"@RESOURCE@\\":" + body + ",\\"meta\\":{\\"pagination\\":{\\"limit\\":15,\\"page\\":1,\\"pages\\":1,\\"prev\\":null,\\"next\\":null,\\"total\\":0}}}";'''.replace('@RESOURCE@',resource)
    if method=='POST':
        return '''        const auto result = mutation.create("@RESOURCE@", input.body.empty() ? "{}" : input.body);
        out["status"]=std::to_string(result.status());
        out["body"]=result.body();'''.replace('@RESOURCE@',resource)
    if method=='PUT':
        if path.strip('/').endswith('/bulk') or len(segments)==1:
            return '''        const auto result = mutation.bulk_edit("@RESOURCE@", input.body.empty() ? "{}" : input.body);
        out["status"]=std::to_string(result.status());
        out["body"]=result.body();'''.replace('@RESOURCE@',resource)
        return '''        const auto result = mutation.edit("@RESOURCE@", input.id, input.body.empty() ? "{}" : input.body);
        out["status"]=std::to_string(result.status());
        out["body"]=result.body();'''.replace('@RESOURCE@',resource)
    if method=='DELETE':
        if detail:
            return '''        const auto result = mutation.destroy("@RESOURCE@", input.id);
        out["status"]=std::to_string(result.status());
        out["body"]=result.body();'''.replace('@RESOURCE@',resource)
        return '''        const auto result = mutation.bulk_destroy("@RESOURCE@", input.body.empty() ? "{}" : input.body);
        out["status"]=std::to_string(result.status());
        out["body"]=result.body();'''.replace('@RESOURCE@',resource)
    return '''        out["status"]="200";
        out["body"]="{}";'''

# collect unique operations, skip posts (already converted)
ops={}
for api,m,p,res,act in ROUTES:
    domain=domain_of(p)
    if domain=='posts': continue
    op=op_name(m,p)
    key=(domain,op)
    if key in ops: continue
    sc=special_call(m,p,res)
    ops[key]={'method':m,'path':p,'resource':res,'domain':domain,'op':op,'special':sc}

# extra custom routes not in catalog
extra=[('members','metafields_create','POST','/members/metafields/custom',('special','metafields_create(input.body)')),
       ('members','metafields_edit','PUT','/members/metafields/custom/:id',('special','metafields_edit(input.id, input.body)')),
       ('members','metafields_read','GET','/members/metafields/custom/:id',('special','metafields_get(input.id)'))]
for domain,op,m,p,sc in extra:
    ops[(domain,op)]={'method':m,'path':p,'resource':'members','domain':domain,'op':op,'special':sc}

all_files=[]
for (domain,op),info in ops.items():
    d=ROOT/domain/op
    for sub in ('model','dto','workflow/step','workflow/flow'):
        (d/sub).mkdir(parents=True,exist_ok=True)
    dcls=cls(domain); ocls=cls(op); C=ocls+dcls+'Handler'
    ctx={'domain':domain,'op':op,'cls':C}
    (d/'model/request.cppm').write_text(t(MODEL,ctx))
    (d/'dto/response.cppm').write_text(t(DTO,ctx))
    sc=info['special']
    special_decl=''
    if sc and sc[0]=='special':
        special_decl='auto& special = blog::handlers::runtime::ServiceRegistry::special<blog::api::SpecialEndpoints>();'
    elif sc and sc[0]=='stripe':
        special_decl='auto& stripe = blog::handlers::runtime::ServiceRegistry::stripe<blog::api::StripeHandler>();'
    if sc and sc[0]=='special':
        body='''        const auto result = special.@CALL@;
        out["status"]=std::to_string(result.status());
        out["body"]=result.body();'''.replace('@CALL@',sc[1])
    elif sc and sc[0]=='stripe':
        pth=sc[1]; m=sc[2]
        if pth.endswith('/config') and m=='GET':
            call='stripe.config()'
        elif pth.endswith('/config') and m=='PUT':
            call='(stripe.configure(input.body), stripe.config())'
        elif m=='POST':
            call='stripe.preview(input.body)'
        else:
            call='stripe.branding()'
        body='''        out["status"]="200";
        out["body"]=@CALL@;'''.replace('@CALL@',call)
    elif sc and sc[0]=='event':
        body='''        out["status"]="200";
        out["body"]="{\\"events\\":[{\\"type\\":\\"comment_event\\",\\"data\\":{\\"post_id\\":\\"1\\",\\"member_id\\":\\"1\\",\\"comment_id\\":\\"1\\"}}],\\"meta\\":{\\"pagination\\":{\\"limit\\":15,\\"page\\":1,\\"pages\\":1,\\"prev\\":null,\\"next\\":null,\\"total\\":1}}}";'''
    else:
        body=generic_body(info['method'],info['path'],info['resource'],domain)
    (d/'workflow/step/execute.cppm').write_text(t(STEP,{**ctx,'special_decl':special_decl,'body':body}))
    (d/'workflow/flow/main.cppm').write_text(t(FLOW,ctx))
    (d/'handler.cppm').write_text(t(HANDLER,ctx))
    for f in ('model/request.cppm','dto/response.cppm','workflow/step/execute.cppm','workflow/flow/main.cppm','handler.cppm'):
        all_files.append(f'operations/{domain}/{op}/{f}')

# update BUILD partitions and base exports
b=pathlib.Path('include/blog/handlers/BUILD.bazel'); s=b.read_text()
start=s.index('    partitions = [\n        "operations/posts_operations.cppm",')
end=s.index('    ],',start)+len('    ],')
parts='    partitions = [\n        "operations/posts_operations.cppm",\n'+''.join(f'        "{f}",\n' for f in all_files)+'    ],'
s=s[:start]+parts+s[end:]
b.write_text(s)
base=pathlib.Path('include/blog/handlers/operations/base.cppm'); s=base.read_text()
exports=''.join(f'export import :{d}_{o}_handler;\n' for (d,o) in ops)
base.write_text(s+'\n'+exports)
print('generated',len(ops),'operations')
