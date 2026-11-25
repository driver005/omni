export module app_handlers_profiles:result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::profiles {

/**
 * Profile-owned wire type for upstream `api_models::admin::ProfileResponse`
 * (`docs/hyperswitch/crates/api_models/src/admin.rs:2838-3062` (v1) /
 * `:3066-...` (v2)). The shared `app::dto::admin::ProfileResponse` keeps its
 * ids as `*Value` wrapper objects, which rfl serializes as `{"value": "..."}`
 * instead of the plain id strings upstream puts on the wire. This module owns
 * its response wire shape, so it derives the shared field bag but overrides
 * the two id accessors with plain `std::string`. Every other field keeps its
 * shared nested type, so no shared DTO has to be edited. Upstream's
 * domain-only extended-card flag (`business_profile.is_extended_card_info_enabled`,
 * `core/admin.rs:4922-4929`) has no `ProfileResponse` field upstream either;
 * the upstream-exposed `extended_card_info_config` field is inherited below
 * and the toggle state stays in the profile's scoped config.
 */
class ProfileWireResponse : public app::dto::admin::ProfileResponse
{
public:
    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_profile_id(std::string value)
    {
        m_profile_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_profile_id() const noexcept
    {
        return m_profile_id;
    }

private:
    std::string m_merchant_id{};
    std::string m_profile_id{};
};

// ───────────────────────── Plain results ─────────────────────────
// Typed results inherit the exact wire shape of the DTO the old core encoded.
// The delete literal and the list arrays are carried as typed data (bool and
// std::vector<ProfileWireResponse>); the handler serializes them with Ser so
// no hand-built JSON text ever reaches the wire.

class ProfileCreateResult : public ProfileWireResponse
{
public:
    ProfileCreateResult() = default;

    explicit ProfileCreateResult(ProfileWireResponse value) noexcept :
        ProfileWireResponse{std::move(value)}
    {
    }
};

class ProfileRetrieveResult : public ProfileWireResponse
{
public:
    ProfileRetrieveResult() = default;

    explicit ProfileRetrieveResult(ProfileWireResponse value) noexcept :
        ProfileWireResponse{std::move(value)}
    {
    }
};

class ProfileUpdateResult : public ProfileWireResponse
{
public:
    ProfileUpdateResult() = default;

    explicit ProfileUpdateResult(ProfileWireResponse value) noexcept :
        ProfileWireResponse{std::move(value)}
    {
    }
};

// core/admin.rs:4195 — Json(bool): the store delete result is a bare `true`.
// The handler serializes get_value() directly to keep the top-level literal.
class ProfileDeleteResult
{
public:
    ProfileDeleteResult() = default;

    explicit ProfileDeleteResult(bool value) noexcept :
        m_value{value}
    {
    }

    [[nodiscard]] bool get_value() const noexcept
    {
        return m_value;
    }

    void set_value(bool value)
    {
        m_value = value;
    }

private:
    bool m_value{true};
};

// Both list routes return a top-level JSON array. Carry the typed vector; the
// handler serializes get_items() so the wire stays a bare array without any
// hand-built JSON text.
class ProfilesListResult
{
public:
    ProfilesListResult() = default;

    explicit ProfilesListResult(std::vector<ProfileWireResponse> items) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<ProfileWireResponse>& get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<ProfileWireResponse> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<ProfileWireResponse> m_items{};
};

class ProfilesListAtProfileLevelResult
{
public:
    ProfilesListAtProfileLevelResult() = default;

    explicit ProfilesListAtProfileLevelResult(std::vector<ProfileWireResponse> items) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<ProfileWireResponse>& get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<ProfileWireResponse> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<ProfileWireResponse> m_items{};
};

class ToggleConnectorAgnosticMitResult : public app::dto::admin::ConnectorAgnosticMitChoice
{
public:
    ToggleConnectorAgnosticMitResult() = default;

    explicit ToggleConnectorAgnosticMitResult(
        app::dto::admin::ConnectorAgnosticMitChoice value
    ) noexcept :
        ConnectorAgnosticMitChoice{std::move(value)}
    {
    }
};

class ToggleExtendedCardInfoResult : public app::dto::admin::ExtendedCardInfoChoice
{
public:
    ToggleExtendedCardInfoResult() = default;

    explicit ToggleExtendedCardInfoResult(app::dto::admin::ExtendedCardInfoChoice value) noexcept :
        ExtendedCardInfoChoice{std::move(value)}
    {
    }
};

} // namespace app::handlers::profiles

template<>
struct serde::Serializable<app::handlers::profiles::ProfileWireResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::handlers::profiles::ProfileWireResponse::get_merchant_id,
                &app::handlers::profiles::ProfileWireResponse::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::handlers::profiles::ProfileWireResponse::get_profile_id,
                &app::handlers::profiles::ProfileWireResponse::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_name",
                &app::handlers::profiles::ProfileWireResponse::get_profile_name,
                &app::handlers::profiles::ProfileWireResponse::set_profile_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::handlers::profiles::ProfileWireResponse::get_return_url,
                &app::handlers::profiles::ProfileWireResponse::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enable_payment_response_hash",
                &app::handlers::profiles::ProfileWireResponse::get_enable_payment_response_hash,
                &app::handlers::profiles::ProfileWireResponse::set_enable_payment_response_hash,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_response_hash_key",
                &app::handlers::profiles::ProfileWireResponse::get_payment_response_hash_key,
                &app::handlers::profiles::ProfileWireResponse::set_payment_response_hash_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "redirect_to_merchant_with_http_post",
                &app::handlers::profiles::ProfileWireResponse::
                    get_redirect_to_merchant_with_http_post,
                &app::handlers::profiles::ProfileWireResponse::
                    set_redirect_to_merchant_with_http_post,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "webhook_details",
                &app::handlers::profiles::ProfileWireResponse::get_webhook_details,
                &app::handlers::profiles::ProfileWireResponse::set_webhook_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::handlers::profiles::ProfileWireResponse::get_metadata,
                &app::handlers::profiles::ProfileWireResponse::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routing_algorithm",
                &app::handlers::profiles::ProfileWireResponse::get_routing_algorithm,
                &app::handlers::profiles::ProfileWireResponse::set_routing_algorithm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "intent_fulfillment_time",
                &app::handlers::profiles::ProfileWireResponse::get_intent_fulfillment_time,
                &app::handlers::profiles::ProfileWireResponse::set_intent_fulfillment_time,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "frm_routing_algorithm",
                &app::handlers::profiles::ProfileWireResponse::get_frm_routing_algorithm,
                &app::handlers::profiles::ProfileWireResponse::set_frm_routing_algorithm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payout_routing_algorithm",
                &app::handlers::profiles::ProfileWireResponse::get_payout_routing_algorithm,
                &app::handlers::profiles::ProfileWireResponse::set_payout_routing_algorithm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "applepay_verified_domains",
                &app::handlers::profiles::ProfileWireResponse::get_applepay_verified_domains,
                &app::handlers::profiles::ProfileWireResponse::set_applepay_verified_domains,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "session_expiry",
                &app::handlers::profiles::ProfileWireResponse::get_session_expiry,
                &app::handlers::profiles::ProfileWireResponse::set_session_expiry,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_link_config",
                &app::handlers::profiles::ProfileWireResponse::get_payment_link_config,
                &app::handlers::profiles::ProfileWireResponse::set_payment_link_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_connector_details",
                &app::handlers::profiles::ProfileWireResponse::get_authentication_connector_details,
                &app::handlers::profiles::ProfileWireResponse::set_authentication_connector_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "use_billing_as_payment_method_billing",
                &app::handlers::profiles::ProfileWireResponse::
                    get_use_billing_as_payment_method_billing,
                &app::handlers::profiles::ProfileWireResponse::
                    set_use_billing_as_payment_method_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "extended_card_info_config",
                &app::handlers::profiles::ProfileWireResponse::get_extended_card_info_config,
                &app::handlers::profiles::ProfileWireResponse::set_extended_card_info_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "collect_shipping_details_from_wallet_connector",
                &app::handlers::profiles::ProfileWireResponse::
                    get_collect_shipping_details_from_wallet_connector,
                &app::handlers::profiles::ProfileWireResponse::
                    set_collect_shipping_details_from_wallet_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "collect_billing_details_from_wallet_connector",
                &app::handlers::profiles::ProfileWireResponse::
                    get_collect_billing_details_from_wallet_connector,
                &app::handlers::profiles::ProfileWireResponse::
                    set_collect_billing_details_from_wallet_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "always_collect_shipping_details_from_wallet_connector",
                &app::handlers::profiles::ProfileWireResponse::
                    get_always_collect_shipping_details_from_wallet_connector,
                &app::handlers::profiles::ProfileWireResponse::
                    set_always_collect_shipping_details_from_wallet_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "always_collect_billing_details_from_wallet_connector",
                &app::handlers::profiles::ProfileWireResponse::
                    get_always_collect_billing_details_from_wallet_connector,
                &app::handlers::profiles::ProfileWireResponse::
                    set_always_collect_billing_details_from_wallet_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_connector_agnostic_mit_enabled",
                &app::handlers::profiles::ProfileWireResponse::
                    get_is_connector_agnostic_mit_enabled,
                &app::handlers::profiles::ProfileWireResponse::
                    set_is_connector_agnostic_mit_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payout_link_config",
                &app::handlers::profiles::ProfileWireResponse::get_payout_link_config,
                &app::handlers::profiles::ProfileWireResponse::set_payout_link_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "outgoing_webhook_custom_http_headers",
                &app::handlers::profiles::ProfileWireResponse::
                    get_outgoing_webhook_custom_http_headers,
                &app::handlers::profiles::ProfileWireResponse::
                    set_outgoing_webhook_custom_http_headers,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_connector_id",
                &app::handlers::profiles::ProfileWireResponse::get_tax_connector_id,
                &app::handlers::profiles::ProfileWireResponse::set_tax_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_tax_connector_enabled",
                &app::handlers::profiles::ProfileWireResponse::get_is_tax_connector_enabled,
                &app::handlers::profiles::ProfileWireResponse::set_is_tax_connector_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_network_tokenization_enabled",
                &app::handlers::profiles::ProfileWireResponse::get_is_network_tokenization_enabled,
                &app::handlers::profiles::ProfileWireResponse::set_is_network_tokenization_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_auto_retries_enabled",
                &app::handlers::profiles::ProfileWireResponse::get_is_auto_retries_enabled,
                &app::handlers::profiles::ProfileWireResponse::set_is_auto_retries_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "max_auto_retries_enabled",
                &app::handlers::profiles::ProfileWireResponse::get_max_auto_retries_enabled,
                &app::handlers::profiles::ProfileWireResponse::set_max_auto_retries_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "always_request_extended_authorization",
                &app::handlers::profiles::ProfileWireResponse::
                    get_always_request_extended_authorization,
                &app::handlers::profiles::ProfileWireResponse::
                    set_always_request_extended_authorization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_click_to_pay_enabled",
                &app::handlers::profiles::ProfileWireResponse::get_is_click_to_pay_enabled,
                &app::handlers::profiles::ProfileWireResponse::set_is_click_to_pay_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_product_ids",
                &app::handlers::profiles::ProfileWireResponse::get_authentication_product_ids,
                &app::handlers::profiles::ProfileWireResponse::set_authentication_product_ids,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_testing_guard_config",
                &app::handlers::profiles::ProfileWireResponse::get_card_testing_guard_config,
                &app::handlers::profiles::ProfileWireResponse::set_card_testing_guard_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_clear_pan_retries_enabled",
                &app::handlers::profiles::ProfileWireResponse::get_is_clear_pan_retries_enabled,
                &app::handlers::profiles::ProfileWireResponse::set_is_clear_pan_retries_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "force_3ds_challenge",
                &app::handlers::profiles::ProfileWireResponse::get_force_3ds_challenge,
                &app::handlers::profiles::ProfileWireResponse::set_force_3ds_challenge,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_debit_routing_enabled",
                &app::handlers::profiles::ProfileWireResponse::get_is_debit_routing_enabled,
                &app::handlers::profiles::ProfileWireResponse::set_is_debit_routing_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_business_country",
                &app::handlers::profiles::ProfileWireResponse::get_merchant_business_country,
                &app::handlers::profiles::ProfileWireResponse::set_merchant_business_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_pre_network_tokenization_enabled",
                &app::handlers::profiles::ProfileWireResponse::
                    get_is_pre_network_tokenization_enabled,
                &app::handlers::profiles::ProfileWireResponse::
                    set_is_pre_network_tokenization_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_configs",
                &app::handlers::profiles::ProfileWireResponse::get_acquirer_configs,
                &app::handlers::profiles::ProfileWireResponse::set_acquirer_configs,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_config_bucket",
                &app::handlers::profiles::ProfileWireResponse::get_acquirer_config_bucket,
                &app::handlers::profiles::ProfileWireResponse::set_acquirer_config_bucket,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_iframe_redirection_enabled",
                &app::handlers::profiles::ProfileWireResponse::get_is_iframe_redirection_enabled,
                &app::handlers::profiles::ProfileWireResponse::set_is_iframe_redirection_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_category_code",
                &app::handlers::profiles::ProfileWireResponse::get_merchant_category_code,
                &app::handlers::profiles::ProfileWireResponse::set_merchant_category_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_country_code",
                &app::handlers::profiles::ProfileWireResponse::get_merchant_country_code,
                &app::handlers::profiles::ProfileWireResponse::set_merchant_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "dispute_polling_interval",
                &app::handlers::profiles::ProfileWireResponse::get_dispute_polling_interval,
                &app::handlers::profiles::ProfileWireResponse::set_dispute_polling_interval,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_manual_retry_enabled",
                &app::handlers::profiles::ProfileWireResponse::get_is_manual_retry_enabled,
                &app::handlers::profiles::ProfileWireResponse::set_is_manual_retry_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "auto_fallback_capture_method",
                &app::handlers::profiles::ProfileWireResponse::get_auto_fallback_capture_method,
                &app::handlers::profiles::ProfileWireResponse::set_auto_fallback_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "always_enable_overcapture",
                &app::handlers::profiles::ProfileWireResponse::get_always_enable_overcapture,
                &app::handlers::profiles::ProfileWireResponse::set_always_enable_overcapture,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_external_vault_enabled",
                &app::handlers::profiles::ProfileWireResponse::get_is_external_vault_enabled,
                &app::handlers::profiles::ProfileWireResponse::set_is_external_vault_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "external_vault_connector_details",
                &app::handlers::profiles::ProfileWireResponse::get_external_vault_connector_details,
                &app::handlers::profiles::ProfileWireResponse::set_external_vault_connector_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_processor_id",
                &app::handlers::profiles::ProfileWireResponse::get_billing_processor_id,
                &app::handlers::profiles::ProfileWireResponse::set_billing_processor_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_connector_details",
                &app::handlers::profiles::ProfileWireResponse::get_surcharge_connector_details,
                &app::handlers::profiles::ProfileWireResponse::set_surcharge_connector_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_l2_l3_enabled",
                &app::handlers::profiles::ProfileWireResponse::get_is_l2_l3_enabled,
                &app::handlers::profiles::ProfileWireResponse::set_is_l2_l3_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_tokenization_credentials",
                &app::handlers::profiles::ProfileWireResponse::get_network_tokenization_credentials,
                &app::handlers::profiles::ProfileWireResponse::set_network_tokenization_credentials,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_blocking",
                &app::handlers::profiles::ProfileWireResponse::get_payment_method_blocking,
                &app::handlers::profiles::ProfileWireResponse::set_payment_method_blocking,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "profile_response";
    }
};

// Reuse the base DTO descriptors: field names, order and nested types stay
// byte-identical to the DTOs the old core passed to encode_json.
template<>
struct serde::Serializable<app::handlers::profiles::ProfileCreateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::handlers::profiles::ProfileWireResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "profile_create_result";
    }
};

template<>
struct serde::Serializable<app::handlers::profiles::ProfileRetrieveResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::handlers::profiles::ProfileWireResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "profile_retrieve_result";
    }
};

template<>
struct serde::Serializable<app::handlers::profiles::ProfileUpdateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::handlers::profiles::ProfileWireResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "profile_update_result";
    }
};

template<>
struct serde::Serializable<app::handlers::profiles::ToggleConnectorAgnosticMitResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::admin::ConnectorAgnosticMitChoice>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "toggle_connector_agnostic_mit_result";
    }
};

template<>
struct serde::Serializable<app::handlers::profiles::ToggleExtendedCardInfoResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::admin::ExtendedCardInfoChoice>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "toggle_extended_card_info_result";
    }
};
