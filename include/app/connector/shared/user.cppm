module;
#include <simdjson.h>
#include <toml++/toml.hpp>
export module app_connector:model_shared_user;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::user {

class MerchantConnectorAccountIdValue : public serde::Serializer<MerchantConnectorAccountIdValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ProfileIdValue : public serde::Serializer<ProfileIdValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CloneConnectorRequest : public serde::Serializer<CloneConnectorRequest>
{
public:
    void set_source_mca_id(MerchantConnectorAccountIdValue value)
    {
        source_mca_id = std::move(value);
    }

    [[nodiscard]] const MerchantConnectorAccountIdValue& get_source_mca_id() const noexcept
    {
        return source_mca_id;
    }

    void set_destination_profile_id(ProfileIdValue value)
    {
        destination_profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_destination_profile_id() const noexcept
    {
        return destination_profile_id;
    }

    void set_connector_label(std::optional<std::string> value)
    {
        connector_label = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_label() const noexcept
    {
        return connector_label;
    }

    MerchantConnectorAccountIdValue source_mca_id{};
    ProfileIdValue destination_profile_id{};
    std::optional<std::string> connector_label{};
};

/**
 * @brief Typed mirror of `api_models::payments::RedirectionResponse` (v1),
 * returned by `core/user.rs:3004-3054` (`get_sso_auth_url`) through
 * `ApplicationResponse::JsonForRedirection`. Upstream `params` is
 * `Vec<(String, Secret<String>)>` and `headers` is `Vec<(String, String)>`;
 * this route always builds both with `Vec::with_capacity(0)`, so a string
 * vector reproduces the exact `[]` wire value for this endpoint.
 */
class RedirectionResponse : public serde::Serializer<RedirectionResponse>
{
public:
    void set_return_url(std::string value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::string& get_return_url() const noexcept
    {
        return return_url;
    }

    void set_params(std::vector<std::string> value)
    {
        params = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_params() const noexcept
    {
        return params;
    }

    void set_return_url_with_query_params(std::string value)
    {
        return_url_with_query_params = std::move(value);
    }

    [[nodiscard]] const std::string& get_return_url_with_query_params() const noexcept
    {
        return return_url_with_query_params;
    }

    void set_http_method(std::string value)
    {
        http_method = std::move(value);
    }

    [[nodiscard]] const std::string& get_http_method() const noexcept
    {
        return http_method;
    }

    void set_headers(std::vector<std::string> value)
    {
        headers = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_headers() const noexcept
    {
        return headers;
    }

    std::string return_url{};
    std::vector<std::string> params{};
    std::string return_url_with_query_params{};
    std::string http_method{};
    std::vector<std::string> headers{};
};

} // namespace app::model::user

template<>
struct serde::Serializable<app::model::user::CloneConnectorRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "source_mca_id",
                &app::model::user::CloneConnectorRequest::get_source_mca_id,
                &app::model::user::CloneConnectorRequest::set_source_mca_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "destination_profile_id",
                &app::model::user::CloneConnectorRequest::get_destination_profile_id,
                &app::model::user::CloneConnectorRequest::set_destination_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_label",
                &app::model::user::CloneConnectorRequest::get_connector_label,
                &app::model::user::CloneConnectorRequest::set_connector_label,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "clone_connector_request";
    }
};

template<>
struct serde::Serializable<app::model::user::RedirectionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "return_url",
                &app::model::user::RedirectionResponse::get_return_url,
                &app::model::user::RedirectionResponse::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "params",
                &app::model::user::RedirectionResponse::get_params,
                &app::model::user::RedirectionResponse::set_params,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url_with_query_params",
                &app::model::user::RedirectionResponse::get_return_url_with_query_params,
                &app::model::user::RedirectionResponse::set_return_url_with_query_params,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "http_method",
                &app::model::user::RedirectionResponse::get_http_method,
                &app::model::user::RedirectionResponse::set_http_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "headers",
                &app::model::user::RedirectionResponse::get_headers,
                &app::model::user::RedirectionResponse::set_headers,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "redirection_response";
    }
};

namespace serde {

/** @brief Scalar wire form for ProfileIdValue (bare JSON string). */
template<>
struct FieldConverter<app::model::user::ProfileIdValue>
{
    using rfl_type = std::string;

    static simdjson::error_code
    from_simdjson(simdjson::ondemand::value& json_value, app::model::user::ProfileIdValue& out)
    {
        std::string_view text;
        if (auto ec = json_value.get_string().get(text); ec) {
            return ec;
        }
        out.set_value(std::string{text});
        return simdjson::error_code{};
    }

    static std::expected<app::model::user::ProfileIdValue, std::string>
    from_toml(const toml::table& table, std::string_view field_name)
    {
        auto value = table[field_name].value<std::string>();
        if (!value) {
            return std::unexpected{std::format("missing or invalid field '{}'", field_name)};
        }
        app::model::user::ProfileIdValue result;
        result.set_value(std::move(*value));
        return result;
    }

    static rfl_type to_rfl(const app::model::user::ProfileIdValue& value)
    {
        return value.get_value();
    }

    static app::model::user::ProfileIdValue from_rfl(const rfl_type& value)
    {
        app::model::user::ProfileIdValue result;
        result.set_value(value);
        return result;
    }
};

} // namespace serde
