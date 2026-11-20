export module blog_handlers:newsletters_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import interfaces;
import :outbox_shim;

// ---------------------------------------------------------------------------
// Strict-parity support for the newsletters controller. Lives in this already
// centrally registered partition (BUILD.bazel:125) so every other newsletter
// step can `import :newsletters_browse_step;` — no new partition, no BUILD edit.
// Shared outbox adapters make verification mail observable through core_outbox
// (CONTRACTS.md §4) without a real SMTP credential.
// ---------------------------------------------------------------------------
export namespace blog::handlers::newsletters::support {

using Object = serde::Value::Object;
using Array = serde::Value::Array;

inline constexpr std::string_view kDocName{"newsletters"};
inline constexpr std::array<std::string_view, 3> kIncludes{
    "count.posts", "count.members", "count.active_members"};
// Ghost CONTENT_API_NEWSLETTER_FIELDS (api-filter-utils.ts:28-40).
inline constexpr std::array<std::string_view, 11> kContentFields{
    "id", "uuid", "name", "description", "slug", "sender_email",
    "subscribe_on_signup", "visibility", "sort_order", "created_at", "updated_at"};
inline constexpr std::string_view kVerifiable[]{"sender_email", "sender_reply_to"};

[[nodiscard]] inline std::string trim(std::string_view s)
{
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.remove_prefix(1);
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.remove_suffix(1);
    return std::string{s};
}

[[nodiscard]] inline std::string escape(std::string_view s)
{
    std::string out;
    for (const char c : s) {
        if (c == '"' || c == '\\') out.push_back('\\');
        out.push_back(c);
    }
    return out;
}

[[nodiscard]] inline std::string now_iso()
{
    return "2026-01-01T00:00:00.000Z";
}

[[nodiscard]] inline const serde::Value* value_for(const Object* object, std::string_view key)
{
    if (object == nullptr) return nullptr;
    for (const auto& [name, value] : *object) if (name == key) return &value;
    return nullptr;
}

[[nodiscard]] inline std::string field(const Object* object, std::string_view key)
{
    const auto* value = value_for(object, key);
    if (value == nullptr) return {};
    if (const auto* text = std::get_if<std::string>(&value->get())) return *text;
    return {};
}

[[nodiscard]] inline serde::Value value_or_null(const Object* object, std::string_view key)
{
    const auto* value = value_for(object, key);
    return value != nullptr ? *value : serde::Value{};
}

template <typename Range>
[[nodiscard]] inline Object project(const Object* object, const Range& keys)
{
    Object out;
    if (object == nullptr) return out;
    for (const auto key : keys) {
        const auto* value = value_for(object, key);
        if (value != nullptr) out[std::string{key}] = *value;
    }
    return out;
}

[[nodiscard]] inline std::optional<serde::Value> decode(std::string_view json)
{
    return core_serde::JsonCodec{}.decode<serde::Value>(json);
}

[[nodiscard]] inline std::string encode(const serde::Value& value)
{
    return core_serde::JsonCodec{}.encode(value);
}

[[nodiscard]] inline Object first_row(std::string_view json)
{
    const auto parsed = decode(json);
    if (!parsed) return {};
    if (const auto* list = std::get_if<Array>(&parsed->get())) {
        if (!list->empty()) {
            if (const auto* object = std::get_if<Object>(&list->front().get())) return *object;
        }
    }
    return {};
}

[[nodiscard]] inline std::string first_id(std::string_view json)
{
    const auto parsed = decode(json);
    if (!parsed) return {};
    if (const auto* root = std::get_if<Object>(&parsed->get())) {
        for (const auto& [key, value] : *root) {
            (void)key;
            if (const auto* list = std::get_if<Array>(&value.get())) {
                if (!list->empty()) {
                    if (const auto* object = std::get_if<Object>(&list->front().get())) {
                        return field(object, "id");
                    }
                }
            }
        }
    }
    return {};
}

[[nodiscard]] inline std::size_t count_rows(std::string_view json)
{
    const auto parsed = decode(json);
    if (!parsed) return 0;
    if (const auto* list = std::get_if<Array>(&parsed->get())) return list->size();
    return 0;
}

/** Ghost output/default.js: `{"<resource>":[...],"meta":<meta>}`. */
[[nodiscard]] inline std::string wrap(std::string_view resource, std::string_view rows,
                                      std::string_view meta = {})
{
    const auto body = trim(rows);
    std::string list;
    if (body.size() >= 2 && body.front() == '[' && body.back() == ']') {
        list = body;
    } else if (body.size() >= 2 && body.front() == '{' && body.back() == '}') {
        list = "[" + body + "]";
    } else {
        list = "[]";
    }
    std::string out = "{\"" + std::string{resource} + "\":" + list;
    if (!meta.empty()) out += ",\"meta\":" + std::string{meta};
    return out + "}";
}

[[nodiscard]] inline std::string meta_pagination(std::size_t total,
                                                 const core::query::ParsedQuery& q)
{
    return "{\"pagination\":" + core::query::pagination_meta(total, q) + "}";
}

[[nodiscard]] inline std::string validation(std::string_view message, std::string_view context,
                                             std::string_view property)
{
    return blog::api::ApiError{}.validation(message, context, property);
}

[[nodiscard]] inline std::string bad_request(std::string_view message)
{
    return blog::api::ApiError{}.validation(message, {}, {}, "BadRequestError");
}

[[nodiscard]] inline std::string not_found(std::string_view message)
{
    return blog::api::ApiError{}.json(message, "Not Found");
}

[[nodiscard]] inline bool valid_email(std::string_view value)
{
    const auto at = value.find('@');
    if (at == std::string_view::npos || at == 0 || at + 1 >= value.size()) return false;
    if (value.find(' ') != std::string_view::npos) return false;
    const auto domain = value.substr(at + 1);
    return domain.find('.') != std::string_view::npos && domain.front() != '.' && domain.back() != '.';
}

[[nodiscard]] inline bool managed_email_enabled(blog::BlogContext& ctx)
{
    const auto settings = ctx.store().json_for("settings", {});
    return settings.find("hostSettings:managedEmail:enabled") != std::string::npos &&
           settings.find("true") != std::string::npos;
}

[[nodiscard]] inline std::size_t next_sort_order(blog::BlogContext& ctx)
{
    std::size_t highest{0};
    const auto parsed = decode(ctx.store().json_for("newsletters", {}));
    if (parsed) {
        if (const auto* list = std::get_if<Array>(&parsed->get())) {
            for (const auto& item : *list) {
                if (const auto* object = std::get_if<Object>(&item.get())) {
                    const std::string raw = field(object, "sort_order");
                    try {
                        if (!raw.empty()) highest = std::max(highest, static_cast<std::size_t>(std::stoll(raw)));
                    } catch (...) {
                    }
                }
            }
        }
    }
    return highest + 1;
}

/** Embed the Ghost `count.*` relations requested through `include`. */
[[nodiscard]] inline std::string embed_counts(std::string_view rows,
                                               const std::set<std::string>& include,
                                               std::size_t posts, std::size_t members,
                                               std::size_t active_members)
{
    if (include.empty()) return std::string{rows};
    std::string extra;
    if (include.contains("count.posts")) extra += "\"posts\":" + std::to_string(posts) + ",";
    if (include.contains("count.members")) extra += "\"members\":" + std::to_string(members) + ",";
    if (include.contains("count.active_members")) extra += "\"active_members\":" + std::to_string(active_members) + ",";
    if (extra.empty()) return std::string{rows};
    extra.pop_back();

    std::string out{rows};
    std::size_t pos{0};
    while ((pos = out.find("\"count\":{", pos)) != std::string::npos) {
        const auto at = pos + 9;
        out.insert(at, extra + ",");
        pos = at + extra.size() + 1;
    }
    return out;
}

/** Project stored rows onto Ghost's 11-field Content API newsletter DTO. */
[[nodiscard]] inline std::string content_dto(std::string_view rows)
{
    auto parsed = decode(rows);
    if (!parsed) return std::string{rows};
    if (auto* list = std::get_if<Array>(&parsed->get())) {
        for (auto& item : *list) {
            if (auto* object = std::get_if<Object>(&item.get())) {
                item = serde::Value{project(object, kContentFields)};
            }
        }
        return encode(*parsed);
    }
    return std::string{rows};
}

/** Split on a delimiter at bracket depth zero and outside quotes. */
[[nodiscard]] inline std::vector<std::string> split_top(std::string_view s, char delim)
{
    std::vector<std::string> out;
    std::size_t start{0};
    int depth{0};
    char quote{0};
    for (std::size_t i = 0; i < s.size(); ++i) {
        const char c = s[i];
        if (quote != 0) {
            if (c == quote) quote = 0;
            continue;
        }
        if (c == '"' || c == '\'') quote = c;
        else if (c == '[') ++depth;
        else if (c == ']' && depth > 0) --depth;
        else if (c == delim && depth == 0) {
            out.push_back(trim(s.substr(start, i - start)));
            start = i + 1;
        }
    }
    out.push_back(trim(s.substr(start)));
    return out;
}

[[nodiscard]] inline bool content_field_allowed(std::string_view field)
{
    std::string f{trim(field)};
    if (f.starts_with("newsletters.")) f.erase(0, 12);
    while (!f.empty() && (f.front() == '-' || f.front() == '!')) f.erase(f.begin());
    return std::ranges::any_of(kContentFields, [&](auto v) { return f == v; });
}

/**
 * Ghost's Content API transformers silently DROP restricted filter statements
 * and order clauses (api-filter-utils.ts:214-216, 343-351); they do not error.
 */
inline void sanitize_content_query(core::query::ParsedQuery& q)
{
    std::erase_if(q.filters, [](const auto& pair) { return !content_field_allowed(pair.first); });

    if (!q.raw_filter.empty()) {
        std::string kept;
        for (const auto& group : split_top(q.raw_filter, ',')) {
            std::string kg;
            for (const auto& clause : split_top(group, '+')) {
                const auto colon = clause.find(':');
                const std::string name = colon == std::string::npos
                                             ? clause : clause.substr(0, colon);
                if (!content_field_allowed(name)) continue;
                kg += (kg.empty() ? "" : "+") + clause;
            }
            if (!kg.empty()) kept += (kept.empty() ? "" : ",") + kg;
        }
        q.raw_filter = kept;
    }
    if (!q.order.empty()) {
        std::string kept;
        for (const auto& clause : split_top(q.order, ',')) {
            const auto sp = clause.find_first_of(" \t");
            const std::string name = sp == std::string::npos ? clause : clause.substr(0, sp);
            if (!content_field_allowed(name)) continue;
            kept += (kept.empty() ? "" : ",") + clause;
        }
        q.order = kept;
    }
    for (auto it = q.fields.begin(); it != q.fields.end();) {
        if (!content_field_allowed(*it)) it = q.fields.erase(it);
        else ++it;
    }
}

[[nodiscard]] inline bool valid_include(const core::query::ParsedQuery& q)
{
    for (const auto& include : q.include) {
        if (!std::ranges::any_of(kIncludes, [&](auto v) { return include == v; })) return false;
    }
    return true;
}

[[nodiscard]] inline std::string request_context(const interfaces::io::IRequest& in)
{
    return std::string{in.find_header("x-blog-context")};
}

// ---------------------------------------------------------------------------
// Outbox adapters. The observable verification-mail side effect is an
// `email.send_verification` event dispatched into the context-local `emails`
// resource; verification acknowledgement is `email.sender_verified`.
// ---------------------------------------------------------------------------
namespace mail_side_effect {
using Event = blog::handlers::outbox::Event;
inline std::vector<Event>& log() { static std::vector<Event> v; return v; }
inline blog::BlogContext*& ctx_ptr() { static blog::BlogContext* p{nullptr}; return p; }

class Adapter final : public blog::handlers::outbox::Adapter {
public:
    void dispatch(const Event& event) override
    {
        if (event.type != "email.send_verification") return;
        log().push_back(event);
        if (ctx_ptr() != nullptr) ctx_ptr()->store().resources().create("emails", event.payload);
    }
};

inline void ensure()
{
    static const bool registered = [] {
        blog::handlers::outbox::register_adapter(std::make_shared<Adapter>());
        return true;
    }();
    static_cast<void>(registered);
}
} // namespace mail_side_effect

namespace verification_side_effect {
using Event = blog::handlers::outbox::Event;
inline std::vector<Event>& log() { static std::vector<Event> v; return v; }

class Adapter final : public blog::handlers::outbox::Adapter {
public:
    void dispatch(const Event& event) override
    {
        if (event.type == "email.sender_verified") log().push_back(event);
    }
};

inline void ensure()
{
    static const bool registered = [] {
        blog::handlers::outbox::register_adapter(std::make_shared<Adapter>());
        return true;
    }();
    static_cast<void>(registered);
}
} // namespace verification_side_effect

/** Issue the fixture token that `verifyPropertyUpdate` later consumes. */
[[nodiscard]] inline std::string issue_verification_token(blog::BlogContext& ctx,
                                                          std::string_view id,
                                                          std::string_view property,
                                                          std::string_view email)
{
    const std::string token = "verify-" + std::string{property} + "-" + std::string{id};
    // ResourceStore overwrites `id` with its own key, so the newsletter id is
    // stored under `newsletter_id` and remapped by consume_verification_token().
    static_cast<void>(ctx.store().resources().create("newsletter_verification_tokens",
        "{\"token\":\"" + escape(token) + "\",\"newsletter_id\":\"" + escape(id) +
        "\",\"property\":\"" + escape(property) + "\",\"value\":\"" + escape(email) + "\"}"));
    return token;
}

[[nodiscard]] inline std::optional<Object> consume_verification_token(blog::BlogContext& ctx,
                                                                      std::string_view token)
{
    const auto parsed = decode(ctx.store().resources().list("newsletter_verification_tokens"));
    if (!parsed) return std::nullopt;
    if (const auto* list = std::get_if<Array>(&parsed->get())) {
        for (const auto& item : *list) {
            if (const auto* object = std::get_if<Object>(&item.get())) {
                if (field(object, "token") != token) continue;
                Object payload;
                for (const auto& [key, value] : *object) {
                    if (key == "token") continue;
                    if (key == "newsletter_id") {
                        payload["id"] = value;
                        continue;
                    }
                    payload[key] = value;
                }
                static_cast<void>(ctx.store().resources().destroy(
                    "newsletter_verification_tokens", field(object, "id")));
                return payload;
            }
        }
    }
    return std::nullopt;
}

} // namespace blog::handlers::newsletters::support

export namespace blog::handlers::newsletters::browse::workflow {

class ExecuteStep {
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"newsletters_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        auto& ctx = *s_ctx;
        const std::string context = request_context(in);
        const bool content = context == "CONTENT_API";

        constexpr std::array<std::string_view, 6> kAdminOptions{
            "include", "filter", "fields", "limit", "order", "page"};
        constexpr std::array<std::string_view, 5> kContentOptions{
            "filter", "fields", "limit", "order", "page"};
        auto q = content ? core::query::parse(in.get_path(), kContentOptions)
                         : core::query::parse(in.get_path(), kAdminOptions);

        // Ghost validation.options.include.values (newsletters.js:14-20).
        if (!content && !valid_include(q)) {
            return core::workflow::StepResponse<Output>{{{"status", "422"},
                {"body", validation("Validation error", "Invalid include", "include")}}};
        }
        if (content) sanitize_content_query(q);
        if (q.order.empty()) q.order = "sort_order asc,created_at asc,id asc";

        std::string available = ctx.store().json_for("newsletters", {});
        if (content) {
            // NewsletterModel.enforcedFilters() => status:active for public context.
            core::query::ParsedQuery active;
            active.raw_filter = "status:active";
            active.limit = std::numeric_limits<std::size_t>::max();
            try {
                available = core::query::apply(available, active);
            } catch (const std::runtime_error&) {
                return core::workflow::StepResponse<Output>{{{"status", "400"},
                    {"body", bad_request("Invalid filter")}}};
            }
        }
        if (content) available = content_dto(available);
        available = embed_counts(available, q.include, ctx.store().total("posts"),
                                 ctx.store().total("members"), ctx.store().total("members"));

        auto count_q = q;
        count_q.limit = std::numeric_limits<std::size_t>::max();
        count_q.page = 1;
        count_q.offset = 0;
        count_q.fields.clear();
        std::string filtered;
        std::string rows;
        try {
            filtered = core::query::apply(available, count_q);
            rows = core::query::apply(available, q);
        } catch (const std::runtime_error&) {
            return core::workflow::StepResponse<Output>{{{"status", "400"},
                {"body", bad_request("Restricted fields cannot be used in order.")}}};
        }
        const std::size_t total = count_rows(filtered);
        const std::string body = wrap("newsletters", rows, meta_pagination(total, q));
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::newsletters::browse::workflow
