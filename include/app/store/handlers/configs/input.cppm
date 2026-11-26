export module app_store:configs_input;

import std;
import interfaces;
import core_serde;
import core_error;
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
 * Kept free of business logic: it only turns request data into the plain
 * Input structs consumed by the domain runtime. Auth and path failures stay
 * in the handler's single response-writing path; the original configs route
 * reported a missing path segment as an empty key, which is preserved here.
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

    [[nodiscard]] bool is_v2(interfaces::io::IRequest& req) const
    {
        return ::app::http::RequestPath{}(req).at(0).value_or("") == "v2";
    }

    [[nodiscard]] std::string config_key_from_request(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        // v1 scope is /configs/:key; v2 scope is /v2/configs/:key.
        return std::string{segments.at(is_v2(req) ? 2U : 1U).value_or("")};
    }

private:
    ::app::context::AppContext& m_context;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old inline core block received.

struct ConfigKeyCreateInput
{
    ConfigRequestWire request{};
};

struct ConfigKeyRetrieveInput
{
    std::string config_key{};
};

struct ConfigKeyUpdateInput
{
    ConfigUpdateRequestWire request{};
    std::string config_key{};
};

struct ConfigKeyDeleteInput
{
    std::string config_key{};
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
