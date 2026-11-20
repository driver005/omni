# Shared abstraction contracts (Phase 0)

All module agents must use these surfaces instead of local stand-ins. Each owner
creates its own directory with its own `BUILD` file; main agent wires dependencies.

## 1. `core_query` — Ghost NQL / input pipeline
Target `//include/core/query:core_query`, module `core_query`.
New dir: `include/core/query/`.

```cpp
export module core_query;
export namespace core::query {

struct ParsedQuery {
    std::map<std::string,std::string> options;      // allowlisted Ghost options
    std::vector<std::pair<std::string,std::string>> filters; // field,value pairs
    std::string raw_filter;                          // bare `filter` NQL
    std::string order;                               // e.g. "created_at desc"
    std::size_t limit{15};
    std::size_t page{1};
    std::size_t offset{0};
    std::set<std::string> fields;                    // empty = all
    std::set<std::string> include;                   // relations to eager-load
};

[[nodiscard]] ParsedQuery parse(std::string_view path,
    std::span<const std::string_view> allowed_options);
[[nodiscard]] bool has_restricted_field(std::string_view nql); // password|token|gift_link_token|secret
[[nodiscard]] bool valid_option(std::string_view key, std::string_view value); // Ghost regex/allowlist checks
[[nodiscard]] std::string apply(std::string_view json_array, const ParsedQuery& q,
    std::size_t default_limit = 15);                // filtered/sorted/projected JSON array
[[nodiscard]] std::string pagination_meta(std::size_t total, const ParsedQuery& q,
    std::size_t default_limit = 15);                // JSON meta.pagination object
[[nodiscard]] std::string run(std::string_view json_array, const ParsedQuery& q,
    std::size_t default_limit = 15);                // {"data":[...], "meta":{...}}

}
```

Requirements: `filter[x]=y` and bare `filter=status:draft,status:published` /
`status:[a,b]` / `field:null` / quoted values; `+` = AND, `,` = OR by Ghost
convention; restricted-field guard throws/flags before filtering; `fields`
projects object keys; `include` is preserved for the mapper; order supports
`field asc|desc`; `limit=all` honored; `page`/`offset` honored.

## 2. `core_auth:ghost` — Ghost permission service
Owner extends `include/core/auth/` (no new target; add partition `ghost.cppm`
to `core_auth/BUILD`, main agent approves). Module `core_auth:ghost`.

```cpp
export module core_auth:ghost;
export import core_auth;
export namespace core::auth::ghost {

enum class ApiContext { ADMIN_API, CONTENT_API, MEMBERS_APP, INTERNAL, SYSTEM };

struct Principal {
    ApiContext context{ApiContext::ANONYMOUS}; // add ANONYMOUS to enum
    std::string id;
    std::string role;                           // Administrator, Editor, etc.
    std::set<std::string> permissions;          // e.g. "posts:browse", "settings:edit"
    bool staff{false};
};

struct AuthResult { bool allowed{false}; int status{200}; std::string error_json; };

class GhostAuthorizer {
public:
    void add_role(std::string name, std::set<std::string> permissions, bool staff = false);
    [[nodiscard]] AuthResult authorize(const Principal&, std::string_view docName,
        std::string_view method, std::string_view id = {}) const;
    [[nodiscard]] AuthResult unsafe_attrs(const Principal&, std::string_view attr,
        std::string_view value) const;
};

}
```

Default roles: Administrator (all), Owner (all), Editor (content + limited
admin), Writer, Contributor, Anonymous. Error envelope must be Ghost-shaped:
`{"errors":[{...,"message":"You do not have permission to browse X","type":"NoPermissionError"}]}`.
`permissions:true`, `false`, `{docName, method}` all route through `authorize`.

## 3. `blog_handlers_middleware` permission middleware
Owner extends `include/blog/handlers/middleware/` (add `ghost_permission.cppm`
partition + export in `middleware/base.cppm`; main agent approves).

```cpp
export namespace blog::handlers::middleware {
inline void require_ghost_permission(std::string_view docName, std::string_view method);
}
```

Reads `x-blog-context`, `x-blog-role`, `x-blog-permissions`, `x-blog-staff`,
`Authorization` headers via `IRequest`; uses `core::auth::ghost::GhostAuthorizer`;
on denial writes Ghost 403 envelope through `write_error`.

## 4. `core_outbox` — events / jobs / side effects
Owner creates `include/core/outbox/` target `//include/core/outbox:core_outbox`,
module `core_outbox`. Reuse `core_events` where practical.

```cpp
export module core_outbox;
export namespace core::outbox {

struct Event { std::string type; std::string payload; std::string aggregate_id; };
class Outbox {
public:
    void emit(Event);
    [[nodiscard]] std::vector<Event> drain();
    void begin_transaction(); void commit(); void rollback();
    [[nodiscard]] std::size_t pending() const noexcept;
};
struct Adapter { virtual ~Adapter() = default; virtual void dispatch(const Event&) = 0; };
void register_adapter(std::shared_ptr<Adapter>);

}
```

Transactions are context-local and observable through `drain()` (no external DB).

## 5. `core_adapters` — real external adapters
Owner creates `include/core/adapters/` target `//include/core/adapters:core_adapters`,
module `core_adapters`. Use `core_client` builder + env vars. No live credentials
required: verify against local mock servers via env `STRIPE_BASE_URL`,
`TINYBIRD_BASE_URL`, `SMTP_URL`, `WEBMENTION_ENDPOINT`.

```cpp
export module core_adapters;
export namespace core::adapters {

struct Config { std::string base_url, api_key, workspace_id, from_address; };
[[nodiscard]] Config from_env(std::string_view prefix);

class StripeAdapter { /* create_session, configure, branding, preview -> JSON string */ };
class TinybirdAdapter { /* token, stats pipe queries -> JSON string */ };
class SmtpAdapter { /* send(message) -> bool */ };
class WebmentionAdapter { /* fetch/source/target -> JSON string */ };

}
```

Each adapter returns the Ghost-shaped JSON body (or a documented error envelope)
and records the outbound call in `core_outbox` for verification. HTTP failures
must produce Ghost-shaped errors, not crashes.

## 6. `blog_handlers:mapping` — output mappers / clean layer
Owner creates `include/blog/handlers/mapping/` with partitions, wired into
`blog_handlers` BUILD by main agent.

```cpp
export module blog_handlers:mapping;
export import :clean;
export namespace blog::mapping {

[[nodiscard]] std::string clean(std::string_view json_object, std::string_view entity);
[[nodiscard]] std::string wrap(std::string_view resource, std::string_view rows_json,
    std::string_view meta_json = {});
[[nodiscard]] std::string pagination(std::size_t total, std::size_t limit,
    std::size_t page, std::size_t pages, std::string_view prev, std::string_view next);

}
```

`clean` applies Ghost field allow-lists per entity (post, user, member, tag,
author, comment, action, tier, offer, webhook) and aliases (`profile_image`,
etc.). `wrap` produces `{"<resource>":[...],"meta":...}`.

## Rules for all agents
- No changes to shared files (`all.cppm`, central `routes.cppm`, root `BUILD.bazel`);
  main agent wires everything.
- Own directory's `BUILD` file is yours.
- Use `std::` + core abstractions only; no third-party libs beyond existing deps.
- Add a small unit test in your own dir where practical.
