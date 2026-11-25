export module blog_handlers:site_browse_step;
import :domain_all;
import :site_dto_site_response;
import std;
import core_workflow;
import core_serde;
import interfaces;

namespace {

using Value = serde::Value;
using Object = Value::Object;
using Records = std::vector<blog::api::RecordDto>;

/** @brief First non-empty environment variable (nconf reads config from env). */
[[nodiscard]] std::string env_first(std::initializer_list<std::string_view> names)
{
    for (const std::string_view name: names) {
        const char* value = std::getenv(std::string{name}.c_str()); // NOLINT(concurrency-mt-unsafe)
        if (value != nullptr && *value != '\0') {
            return std::string{value};
        }
    }
    return {};
}

/** @brief Stored values keep their JSON quoting; strip surrounding quotes. */
[[nodiscard]] std::string unquote(std::string_view value)
{
    if (value.size() >= 2u && value.front() == '"' && value.back() == '"') {
        return std::string{value.substr(1u, value.size() - 2u)};
    }
    return std::string{value};
}

/** @brief One key from a context-local key/value namespace (config or settings). */
[[nodiscard]] std::optional<std::string> raw_setting(const Records& rows, std::string_view key)
{
    for (const auto& record: rows) {
        if (const auto it = record.find("key"); it != record.end() && unquote(it->second) == key) {
            if (const auto value = record.find("value"); value != record.end()) {
                return unquote(value->second);
            }
        }
        if (const auto it = record.find(std::string{key}); it != record.end()) {
            return unquote(it->second);
        }
    }
    return std::nullopt;
}

/** @brief Stored values are JSON text; a bare word is a plain string (nconf style). */
[[nodiscard]] Value to_value(std::string_view raw)
{
    if (const auto decoded = core_serde::JsonCodec{}.decode<Value>(raw)) {
        return *decoded;
    }
    return Value{unquote(raw)};
}

/** @brief Context-local config namespace wins over settings, mirroring config.get precedence. */
[[nodiscard]] std::optional<Value>
lookup(const Records& config, const Records& settings, std::string_view key)
{
    if (const auto raw = raw_setting(config, key)) {
        return to_value(*raw);
    }
    if (const auto raw = raw_setting(settings, key)) {
        return to_value(*raw);
    }
    return std::nullopt;
}

/** @brief rfl::Object keeps find() private; count()+at() are the public lookup pair. */
[[nodiscard]] const Value* field(const Value* value, std::string_view key)
{
    if (value == nullptr) {
        return nullptr;
    }
    const auto* object = std::get_if<Object>(&value->get());
    if (object == nullptr) {
        return nullptr;
    }
    const std::string name{key};
    return object->count(name) == 0 ? nullptr : &object->at(name);
}

[[nodiscard]] bool truthy(const Value& value)
{
    if (const auto* flag = std::get_if<bool>(&value.get())) {
        return *flag;
    }
    if (const auto* text = std::get_if<std::string>(&value.get())) {
        return *text == "true" || *text == "1" || *text == "yes";
    }
    return false;
}

[[nodiscard]] std::string text_or(const std::optional<Value>& value, std::string fallback)
{
    if (value) {
        if (const auto* text = std::get_if<std::string>(&value->get())) {
            return *text;
        }
    }
    return fallback;
}

/** @brief nconf boolean: JSON bool or "true"/"false" string; fallback when absent. */
[[nodiscard]] bool bool_or(const std::optional<Value>& value, bool fallback)
{
    if (!value) {
        return fallback;
    }
    if (const auto* flag = std::get_if<bool>(&value->get())) {
        return *flag;
    }
    if (const auto* text = std::get_if<std::string>(&value->get())) {
        if (*text == "true" || *text == "1" || *text == "yes") {
            return true;
        }
        if (*text == "false" || *text == "0" || *text == "no") {
            return false;
        }
    }
    return fallback;
}

/** @brief Ghost urlUtils.urlFor('home', true): configured absolute home URL, trailing slash. */
[[nodiscard]] std::string
home_url(const Records& config, const Records& settings, interfaces::io::IRequest& in)
{
    std::string home = env_first({"URL"});
    if (home.empty()) {
        home = text_or(lookup(config, settings, "url"), {});
    }
    if (home.empty()) {
        const auto scheme = in.get_scheme();
        const auto authority = in.get_authority();
        if (!scheme.empty() && !authority.empty()) {
            home = std::string{scheme} + "://" + std::string{authority} + "/";
        }
    }
    if (home.empty()) {
        home = "http://127.0.0.1:2369/";
    }
    if (!home.ends_with('/')) {
        home.push_back('/');
    }
    return home;
}

/** @brief Conditional sentry_dsn/sentry_env from config.get('client_sentry'). */
void add_sentry(Object& out, const Records& config, const Records& settings)
{
    const auto sentry = lookup(config, settings, "client_sentry");
    if (!sentry) {
        return;
    }

    bool disabled = false;
    if (const auto* flag = field(&*sentry, "disabled")) {
        disabled = truthy(*flag);
    }
    if (const auto flat = lookup(config, settings, "client_sentry:disabled")) {
        disabled = truthy(*flat);
    }
    if (disabled) {
        return;
    }

    std::string dsn;
    if (const auto* value = field(&*sentry, "dsn")) {
        dsn = text_or(std::optional<Value>{*value}, {});
    }
    if (dsn.empty()) {
        dsn = text_or(lookup(config, settings, "client_sentry:dsn"), {});
    }
    out["sentry_dsn"] = Value{dsn};

    std::string environment = env_first({"PRO_ENV"});
    if (environment.empty()) {
        environment = text_or(lookup(config, settings, "PRO_ENV"), {});
    }
    if (environment.empty()) {
        environment = text_or(lookup(config, settings, "env"), {});
    }
    out["sentry_env"] = Value{environment};
}

/** @brief Reproduces getSiteProperties() + output/site.js#read (14-key pick, no meta). */
[[nodiscard]] Object build_site(blog::BlogContext& ctx, interfaces::io::IRequest& in)
{
    const Records config = blog::api::parse_array(ctx.store().resources().list("config"));
    const Records settings = blog::api::parse_array(ctx.store().json_for("settings", {}));
    const auto fallback = blog::dto::site::SiteResponse{};

    Object out;

    // settingsCache.get(...) — eight values are present in settings_json().
    out["title"] = Value{text_or(lookup(config, settings, "title"), fallback.get_title())};
    out["description"] =
        Value{text_or(lookup(config, settings, "description"), fallback.get_description())};
    out["logo"] = Value{text_or(lookup(config, settings, "logo"), {})};
    out["icon"] = Value{text_or(lookup(config, settings, "icon"), {})};
    out["cover_image"] =
        Value{text_or(lookup(config, settings, "cover_image"), fallback.get_cover_image())};
    out["accent_color"] =
        Value{text_or(lookup(config, settings, "accent_color"), fallback.get_accent_color())};
    out["locale"] = Value{text_or(lookup(config, settings, "locale"), fallback.get_locale())};
    out["timezone"] = Value{text_or(lookup(config, settings, "timezone"), fallback.get_timezone())};

    // urlUtils.urlFor('home', true) — configured home URL, not the request Host header.
    out["url"] = Value{home_url(config, settings, in)};

    // ghostVersion.safe — GHOST_BUILD_VERSION wins, then the store, then the harness default.
    std::string version = env_first({"GHOST_BUILD_VERSION"});
    if (version.empty()) {
        version = text_or(lookup(config, settings, "version"), fallback.get_version());
    }
    out["version"] = Value{version};

    // allow_external_signup = allow_self_signup && !(checkbox_required && terms_html).
    const bool self_signup = bool_or(
        lookup(config, settings, "allow_self_signup"),
        fallback.get_allow_external_signup()
    );
    const bool checkbox =
        bool_or(lookup(config, settings, "portal_signup_checkbox_required"), false);
    const std::string terms = text_or(lookup(config, settings, "portal_signup_terms_html"), {});
    out["allow_external_signup"] = Value{self_signup && !(checkbox && !terms.empty())};

    // site_uuid from settingsCache.get('site_uuid'), with the harness default as fallback.
    out["site_uuid"] =
        Value{text_or(lookup(config, settings, "site_uuid"), fallback.get_site_uuid())};

    add_sentry(out, config, settings);

    return out;
}

} // namespace

export namespace blog::handlers::site::browse::workflow {

/** @brief Shared Ghost site body: {"site": <14-key pick>}, no meta. */
[[nodiscard]] std::string build_site_body(blog::BlogContext& ctx, interfaces::io::IRequest& in)
{
    Object envelope;
    envelope["site"] = Value{build_site(ctx, in)};
    return core_serde::JsonCodec{}.encode(Value{std::move(envelope)});
}

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"site_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        return core::workflow::StepResponse<Output>{build_site_body(*s_ctx, in)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::site::browse::workflow
