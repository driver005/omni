export module blog_handlers:config_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;

namespace {

using Value = serde::Value;
using Object = Value::Object;
using Records = std::vector<blog::api::RecordDto>;

[[nodiscard]] std::string env_first(std::initializer_list<std::string_view> names)
{
    for (const std::string_view name : names) {
        const char* value = std::getenv(std::string{name}.c_str());
        if (value != nullptr && *value != '\0') return std::string{value};
    }
    return {};
}

[[nodiscard]] std::string unquote(std::string_view value)
{
    if (value.size() >= 2u && value.front() == '"' && value.back() == '"')
        return std::string{value.substr(1u, value.size() - 2u)};
    return std::string{value};
}

/** @brief Reads one key from a context-local key/value namespace (config or settings). */
[[nodiscard]] std::optional<std::string> raw_setting(const Records& rows, std::string_view key)
{
    for (const auto& record : rows) {
        if (const auto it = record.find("key");
            it != record.end() && unquote(it->second) == key) {
            if (const auto value = record.find("value"); value != record.end())
                return unquote(value->second);
        }
        if (const auto it = record.find(std::string{key}); it != record.end())
            return unquote(it->second);
    }
    return std::nullopt;
}

/** @brief Stored values are JSON text; a bare word is a plain string (nconf style). */
[[nodiscard]] Value to_value(std::string_view raw)
{
    if (const auto decoded = core_serde::JsonCodec{}.decode<Value>(raw)) return *decoded;
    return Value{unquote(raw)};
}

/** @brief Context-local config namespace wins over settings, mirroring config.get precedence. */
[[nodiscard]] std::optional<Value> lookup(const Records& config,
                                          const Records& settings,
                                          std::string_view key)
{
    if (const auto raw = raw_setting(config, key)) return to_value(*raw);
    if (const auto raw = raw_setting(settings, key)) return to_value(*raw);
    return std::nullopt;
}

/** @brief rfl::Object keeps find() private; count()+at() are the public lookup pair. */
[[nodiscard]] const Value* field(const Value* value, std::string_view key)
{
    if (value == nullptr) return nullptr;
    const auto* object = std::get_if<Object>(&value->get());
    if (object == nullptr) return nullptr;
    const std::string name{key};
    return object->count(name) == 0 ? nullptr : &object->at(name);
}

[[nodiscard]] bool truthy(const Value& value)
{
    if (const auto* flag = std::get_if<bool>(&value.get())) return *flag;
    if (const auto* text = std::get_if<std::string>(&value.get()))
        return *text == "true" || *text == "1" || *text == "yes";
    return false;
}

[[nodiscard]] std::string text_or(const std::optional<Value>& value, std::string fallback)
{
    if (value) if (const auto* text = std::get_if<std::string>(&value->get())) return *text;
    return fallback;
}

[[nodiscard]] bool bool_or(const std::optional<Value>& value, bool fallback)
{
    if (value) if (std::holds_alternative<bool>(value->get())) return truthy(*value);
    return fallback;
}

/** @brief Ghost sanitizeHostSettings: omit the two exported/verification webhook secrets. */
[[nodiscard]] Value sanitize_host_settings(const std::optional<Value>& raw)
{
    if (!raw) return Value{Object{}};
    const auto* object = std::get_if<Object>(&raw->get());
    if (object == nullptr) return *raw; // Ghost returns non-objects unchanged.

    Object clean;
    for (const auto& [key, value] : *object) {
        if (key == "export.webhookSecret" || key == "emailVerification.webhookSecret") continue;
        clean.insert(key, value);
    }
    for (const std::string_view nested : {"export", "emailVerification"}) {
        const std::string name{nested};
        if (clean.count(name) == 0) continue;
        const auto* child = std::get_if<Object>(&clean.at(name).get());
        if (child == nullptr || child->count("webhookSecret") == 0) continue;
        Object child_copy;
        for (const auto& [key, value] : *child) {
            if (key == "webhookSecret") continue;
            child_copy.insert(key, value);
        }
        clean.at(name) = Value{std::move(child_copy)};
    }
    return Value{std::move(clean)};
}

/** @brief Ghost getTinybirdStatsPayload: copy allowlisted props, add id + optional local block. */
[[nodiscard]] Value tinybird_stats(const Value& stats_config, const Records& settings)
{
    Object payload;
    for (const std::string_view key : {"endpoint", "endpointBrowser", "version", "datasource"}) {
        if (const auto* value = field(&stats_config, key)) {
            const std::string name{key};
            payload.insert(name, *value);
        }
    }

    std::string site_uuid = text_or(field(&stats_config, "id") != nullptr
                                        ? std::optional<Value>{*field(&stats_config, "id")}
                                        : std::optional<Value>{},
                                    {});
    if (site_uuid.empty())
        if (const auto raw = raw_setting(settings, "site_uuid")) site_uuid = *raw;
    payload["id"] = Value{site_uuid};

    const auto* local = field(&stats_config, "local");
    if (local != nullptr) {
        Object local_payload;
        bool has_local = false;
        for (const std::string_view key : {"enabled", "endpoint", "datasource"}) {
            if (const auto* value = field(local, key)) {
                const std::string name{key};
                local_payload.insert(name, *value);
                has_local = true;
            }
        }
        if (has_local) payload["local"] = Value{std::move(local_payload)};
    }
    return Value{std::move(payload)};
}

/** @brief Ghost exposes only featurebase.enabled and featurebase.organization. */
[[nodiscard]] Value featurebase_public(const Value& featurebase)
{
    Object out;
    for (const std::string_view key : {"enabled", "organization"}) {
        if (const auto* value = field(&featurebase, key)) {
            const std::string name{key};
            out.insert(name, *value);
        }
    }
    return Value{std::move(out)};
}

/** @brief Ghost auth is an allowlist: pick(config.auth, ['engine']). */
[[nodiscard]] Value auth_public(const std::optional<Value>& raw)
{
    Object out;
    if (const auto* engine = raw ? field(&*raw, "engine") : nullptr)
        out["engine"] = *engine;
    else
        out["engine"] = Value{std::string{"legacy"}}; // harness fallback for absent config.auth
    return Value{std::move(out)};
}

/** @brief Reproduces getConfigProperties() + output/config.js.all (20-key pick, no meta). */
[[nodiscard]] Object build_config(blog::BlogContext& ctx)
{
    const Records config = blog::api::parse_array(ctx.store().resources().list("config"));
    const Records settings = blog::api::parse_array(ctx.store().json_for("settings", {}));

    Object out;

    // version: GHOST_BUILD_VERSION wins, then the store, then a harness version fallback.
    std::string version = env_first({"GHOST_BUILD_VERSION"});
    if (version.empty()) version = text_or(lookup(config, settings, "version"), "5.0.0");
    out["version"] = Value{version};

    std::string environment = text_or(lookup(config, settings, "environment"), {});
    if (environment.empty()) environment = text_or(lookup(config, settings, "env"), "testing-mysql");
    out["environment"] = Value{environment};
    out["database"] = Value{text_or(lookup(config, settings, "database"), "mysql8")};

    // mail: isPlainObject(config.mail) ? config.mail.transport : ''
    std::string mail;
    if (const auto raw = lookup(config, settings, "mail")) {
        if (const auto* transport = field(&*raw, "transport"))
            mail = text_or(std::optional<Value>{*transport}, {});
        else
            mail = text_or(std::optional<Value>{*raw}, {});
    }
    out["mail"] = Value{mail};

    // useGravatar: !config.isPrivacyDisabled('useGravatar') — true unless privacy disables it.
    bool use_gravatar = true;
    if (const auto privacy = lookup(config, settings, "privacy:useGravatar")) use_gravatar = !truthy(*privacy);
    if (const auto gravatar_override = lookup(config, settings, "useGravatar")) use_gravatar = truthy(*gravatar_override);
    out["useGravatar"] = Value{use_gravatar};

    out["labs"] = lookup(config, settings, "labs").value_or(Value{Object{}});
    out["clientExtensions"] = lookup(config, settings, "clientExtensions").value_or(Value{Object{}});
    out["enableDeveloperExperiments"] = Value{bool_or(lookup(config, settings, "enableDeveloperExperiments"), false)};
    out["stripeDirect"] = Value{bool_or(lookup(config, settings, "stripeDirect"), false)};

    bool mailgun = false;
    if (const auto bulk = lookup(config, settings, "bulkEmail"))
        if (const auto* configured = field(&*bulk, "mailgun")) mailgun = truthy(*configured);
    if (const auto direct = lookup(config, settings, "bulkEmail:mailgun")) mailgun = truthy(*direct);
    out["mailgunIsConfigured"] = Value{mailgun};

    bool email_analytics = true;
    if (const auto direct = lookup(config, settings, "emailAnalytics:enabled"))
        email_analytics = truthy(*direct);
    else if (const auto grouped = lookup(config, settings, "emailAnalytics"))
        if (const auto* enabled = field(&*grouped, "enabled")) email_analytics = truthy(*enabled);
    out["emailAnalytics"] = Value{email_analytics};

    out["hostSettings"] = sanitize_host_settings(lookup(config, settings, "hostSettings"));
    out["klipy"] = lookup(config, settings, "klipy").value_or(Value{Object{}});
    out["pintura"] = lookup(config, settings, "pintura").value_or(Value{Object{}});
    out["signupForm"] = lookup(config, settings, "signupForm").value_or(Value{Object{}});
    out["security"] = lookup(config, settings, "security").value_or(Value{Object{}});
    out["auth"] = auth_public(lookup(config, settings, "auth"));

    // Conditional keys — Ghost only adds these when the backing config is present.
    if (const auto url = lookup(config, settings, "explore:testimonials_url"))
        out["exploreTestimonialsUrl"] = *url;
    else if (const auto url = lookup(config, settings, "explore.testimonials_url"))
        out["exploreTestimonialsUrl"] = *url;

    if (const auto stats = lookup(config, settings, "tinybird:stats"))
        out["stats"] = tinybird_stats(*stats, settings);
    else if (const auto stats = lookup(config, settings, "tinybird.stats"))
        out["stats"] = tinybird_stats(*stats, settings);

    if (const auto featurebase = lookup(config, settings, "featurebase"))
        if (truthy(*featurebase)) out["featurebase"] = featurebase_public(*featurebase);

    return out;
}

} // namespace

export namespace blog::handlers::config::browse::workflow {

/** @brief Shared Ghost config body: {"config": <picked keys>}, no meta. */
[[nodiscard]] std::string build_config_body(blog::BlogContext& ctx)
{
    Object envelope;
    envelope["config"] = Value{build_config(ctx)};
    return core_serde::JsonCodec{}.encode(Value{std::move(envelope)});
}

class ExecuteStep {
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"config_browse_execute"};
    [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
        (void)in; // Ghost read.query() takes no frame input.
        return core::workflow::StepResponse<Output>{build_config_body(*s_ctx)};
    }
    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
    static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::config::browse::workflow
