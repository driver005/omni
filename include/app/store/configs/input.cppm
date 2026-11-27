export module app_store:configs_input;

import std;
import interfaces;
import core_serde;
import core_error;
import core_behavior;
import app_context;
import app_context_core;
import app_http;

export namespace app::handlers::configs {

/**
 * Local mirror of upstream `api_types::Config{key: String, value: String}`
 * (`docs/hyperswitch/crates/router/src/types/api/configs.rs:1-5`).
 * Both fields are required strings: reflect-cpp treats a missing or
 * non-string field as a deserialization failure, exactly like serde.
 */
class ConfigRequestWire
{
public:
    void set_key(std::string value)
    {
        key = std::move(value);
    }

    [[nodiscard]] const std::string& get_key() const noexcept
    {
        return key;
    }

    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string key{};
    std::string value{};
};

/**
 * Local mirror of upstream `api_types::ConfigUpdate`
 * (`docs/hyperswitch/crates/router/src/types/api/configs.rs:7-12`).
 * Upstream's `key` is `#[serde(skip_deserializing)]` and is overwritten
 * from the web::Path<String> in the route (routes/configs.rs:61-66), so
 * this wire shape intentionally carries only `value`: any `key` in the
 * body is ignored, exactly as upstream does.
 */
class ConfigUpdateRequestWire
{
public:
    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

/**
 * @brief Parse/auth/path support for the configs handlers.
 *
 * Kept free of business logic: it builds the HTTP-free CallContext consumed by
 * the domain runtime. Auth failures stay in the handler's response-writing path;
 * a missing path segment remains an empty key, as in the original route.
 */
class ConfigsInputSupport
{
public:
    explicit ConfigsInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError>
    authorize_operation(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    /** @brief HTTP-free view of the request for the config sub-runtimes. */
    [[nodiscard]] core::behavior::CallContext call_context(interfaces::io::IRequest& req) const
    {
        core::behavior::CallContext ctx{};
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        ctx.merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (ctx.merchant_id.empty()) {
            ctx.merchant_id = auth.merchant_id();
        }
        ctx.org_id = ::app::http::org_id_from_header{}(req);
        if (ctx.org_id.empty()) {
            ctx.org_id = auth.org_id();
        }
        ctx.profile_id = auth.profile_id();
        if (const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId)) {
            ctx.connected_merchant_id = std::string{*connected};
        }
        if (const auto secret = auth.claim(::app::auth::kClaimClientSecret)) {
            ctx.client_secret = std::string{*secret};
        }
        const auto segments = ::app::http::RequestPath{}(req);
        for (std::size_t i{0}; i < segments.count(); ++i) {
            if (const auto segment = segments.at(i)) {
                ctx.path_segments.emplace_back(*segment);
            }
        }
        const std::string_view raw = req.get_path();
        const auto question = raw.find('?');
        if (question != std::string_view::npos) {
            auto tail = raw.substr(question + 1);
            while (!tail.empty()) {
                const auto amp = tail.find('&');
                const auto pair =
                    tail.substr(0, amp == std::string_view::npos ? tail.size() : amp);
                const auto eq = pair.find('=');
                ctx.query[std::string{
                    eq == std::string_view::npos ? pair : pair.substr(0, eq)
                }] = eq == std::string_view::npos ? std::string{} : std::string{pair.substr(eq + 1)};
                if (amp == std::string_view::npos) {
                    break;
                }
                tail = tail.substr(amp + 1);
            }
        }
        for (const auto& [key, value]: auth.claims()) {
            ctx.claims[key] = value;
        }
        return ctx;
    }

private:
    ::app::context::AppContext& m_context;
};

/**
 * @brief Domain policy consumed by core::behavior::RequestSupport.
 *
 * Config operations resolve their admin rule from the shared registry. The
 * scoped key is a path segment: v1 is /configs/:key, v2 is /v2/configs/:key,
 * so the runtime reads it from CallContext::path_segments.
 */
class ConfigsPolicy
{
public:
    explicit ConfigsPolicy(ConfigsInputSupport& support) noexcept :
        m_support{support}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        (void)spec; // admin rules are registry-driven, keyed by operation.
        return m_support.authorize_operation(req, operation);
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest&) const noexcept
    {
        return {};
    }

    [[nodiscard]] core::behavior::CallContext context(interfaces::io::IRequest& req) const
    {
        return m_support.call_context(req);
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope&) const noexcept
    {
        return std::nullopt;
    }

private:
    ConfigsInputSupport& m_support;
};

} // namespace app::handlers::configs

// Serialization metadata documents the exact upstream wire fields. Decoding is
// done by serde::Ser::deserialize<T>, which uses reflect-cpp on the public fields.
template<>
struct serde::Serializable<::app::handlers::configs::ConfigRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "key",
                &::app::handlers::configs::ConfigRequestWire::get_key,
                &::app::handlers::configs::ConfigRequestWire::set_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value",
                &::app::handlers::configs::ConfigRequestWire::get_value,
                &::app::handlers::configs::ConfigRequestWire::set_value,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "config_request_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::configs::ConfigUpdateRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &::app::handlers::configs::ConfigUpdateRequestWire::get_value,
            &::app::handlers::configs::ConfigUpdateRequestWire::set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "config_update_request_wire";
    }
};
