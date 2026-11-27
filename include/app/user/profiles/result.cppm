export module app_user:profiles_result;

import std;
import core_serde;
import interfaces;
import app_dto;

export namespace app::handlers::profiles {

class ProfileWireResponse :
    public ::app::dto::admin::ProfileResponse,
    public serde::Serializer<ProfileWireResponse>
{
public:
    // ProfileResponse brings its own Serializer<ProfileResponse>::serialize; pin this
    // type's overload so the profile wire fields win at every call site.
    using serde::Serializer<ProfileWireResponse>::serialize;

    void to_response(interfaces::io::ResponseSink& sink) const
    {
        serde::Serializer<ProfileWireResponse>::to_response(sink);
    }

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

// Wire field lists captured from the former Result Serializable specializations.
// Each Result reused the full FieldDesc tuple of its underlying model, so these
// lists hold the exact declared field names in Serializable<T>::fields() order.

inline constexpr std::array<std::string_view, 56> kProfileWireFields{
    "merchant_id",
    "profile_id",
    "profile_name",
    "return_url",
    "enable_payment_response_hash",
    "payment_response_hash_key",
    "redirect_to_merchant_with_http_post",
    "webhook_details",
    "metadata",
    "routing_algorithm",
    "intent_fulfillment_time",
    "frm_routing_algorithm",
    "payout_routing_algorithm",
    "applepay_verified_domains",
    "session_expiry",
    "payment_link_config",
    "authentication_connector_details",
    "use_billing_as_payment_method_billing",
    "extended_card_info_config",
    "collect_shipping_details_from_wallet_connector",
    "collect_billing_details_from_wallet_connector",
    "always_collect_shipping_details_from_wallet_connector",
    "always_collect_billing_details_from_wallet_connector",
    "is_connector_agnostic_mit_enabled",
    "payout_link_config",
    "outgoing_webhook_custom_http_headers",
    "tax_connector_id",
    "is_tax_connector_enabled",
    "is_network_tokenization_enabled",
    "is_auto_retries_enabled",
    "max_auto_retries_enabled",
    "always_request_extended_authorization",
    "is_click_to_pay_enabled",
    "authentication_product_ids",
    "card_testing_guard_config",
    "is_clear_pan_retries_enabled",
    "force_3ds_challenge",
    "is_debit_routing_enabled",
    "merchant_business_country",
    "is_pre_network_tokenization_enabled",
    "acquirer_configs",
    "acquirer_config_bucket",
    "is_iframe_redirection_enabled",
    "merchant_category_code",
    "merchant_country_code",
    "dispute_polling_interval",
    "is_manual_retry_enabled",
    "auto_fallback_capture_method",
    "always_enable_overcapture",
    "is_external_vault_enabled",
    "external_vault_connector_details",
    "billing_processor_id",
    "surcharge_connector_details",
    "is_l2_l3_enabled",
    "network_tokenization_credentials",
    "payment_method_blocking",
};

inline constexpr std::array<std::string_view, 1> kConnectorAgnosticMitChoiceWireFields{
    "enabled",
};

inline constexpr std::array<std::string_view, 1> kExtendedCardInfoChoiceWireFields{
    "enabled",
};


} // namespace app::handlers::profiles

template<>
struct serde::Serializable<::app::handlers::profiles::ProfileWireResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &::app::handlers::profiles::ProfileWireResponse::get_merchant_id,
                &::app::handlers::profiles::ProfileWireResponse::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &::app::handlers::profiles::ProfileWireResponse::get_profile_id,
                &::app::handlers::profiles::ProfileWireResponse::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_name",
                &::app::handlers::profiles::ProfileWireResponse::get_profile_name,
                &::app::handlers::profiles::ProfileWireResponse::set_profile_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &::app::handlers::profiles::ProfileWireResponse::get_return_url,
                &::app::handlers::profiles::ProfileWireResponse::set_return_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "enable_payment_response_hash",
                &::app::handlers::profiles::ProfileWireResponse::get_enable_payment_response_hash,
                &::app::handlers::profiles::ProfileWireResponse::set_enable_payment_response_hash,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_response_hash_key",
                &::app::handlers::profiles::ProfileWireResponse::get_payment_response_hash_key,
                &::app::handlers::profiles::ProfileWireResponse::set_payment_response_hash_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "redirect_to_merchant_with_http_post",
                &::app::handlers::profiles::ProfileWireResponse::
                    get_redirect_to_merchant_with_http_post,
                &::app::handlers::profiles::ProfileWireResponse::
                    set_redirect_to_merchant_with_http_post,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "webhook_details",
                &::app::handlers::profiles::ProfileWireResponse::get_webhook_details,
                &::app::handlers::profiles::ProfileWireResponse::set_webhook_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &::app::handlers::profiles::ProfileWireResponse::get_metadata,
                &::app::handlers::profiles::ProfileWireResponse::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "routing_algorithm",
                &::app::handlers::profiles::ProfileWireResponse::get_routing_algorithm,
                &::app::handlers::profiles::ProfileWireResponse::set_routing_algorithm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "intent_fulfillment_time",
                &::app::handlers::profiles::ProfileWireResponse::get_intent_fulfillment_time,
                &::app::handlers::profiles::ProfileWireResponse::set_intent_fulfillment_time,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "frm_routing_algorithm",
                &::app::handlers::profiles::ProfileWireResponse::get_frm_routing_algorithm,
                &::app::handlers::profiles::ProfileWireResponse::set_frm_routing_algorithm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payout_routing_algorithm",
                &::app::handlers::profiles::ProfileWireResponse::get_payout_routing_algorithm,
                &::app::handlers::profiles::ProfileWireResponse::set_payout_routing_algorithm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "applepay_verified_domains",
                &::app::handlers::profiles::ProfileWireResponse::get_applepay_verified_domains,
                &::app::handlers::profiles::ProfileWireResponse::set_applepay_verified_domains,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "session_expiry",
                &::app::handlers::profiles::ProfileWireResponse::get_session_expiry,
                &::app::handlers::profiles::ProfileWireResponse::set_session_expiry,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_link_config",
                &::app::handlers::profiles::ProfileWireResponse::get_payment_link_config,
                &::app::handlers::profiles::ProfileWireResponse::set_payment_link_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_connector_details",
                &::app::handlers::profiles::ProfileWireResponse::
                    get_authentication_connector_details,
                &::app::handlers::profiles::ProfileWireResponse::
                    set_authentication_connector_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "use_billing_as_payment_method_billing",
                &::app::handlers::profiles::ProfileWireResponse::
                    get_use_billing_as_payment_method_billing,
                &::app::handlers::profiles::ProfileWireResponse::
                    set_use_billing_as_payment_method_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "extended_card_info_config",
                &::app::handlers::profiles::ProfileWireResponse::get_extended_card_info_config,
                &::app::handlers::profiles::ProfileWireResponse::set_extended_card_info_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "collect_shipping_details_from_wallet_connector",
                &::app::handlers::profiles::ProfileWireResponse::
                    get_collect_shipping_details_from_wallet_connector,
                &::app::handlers::profiles::ProfileWireResponse::
                    set_collect_shipping_details_from_wallet_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "collect_billing_details_from_wallet_connector",
                &::app::handlers::profiles::ProfileWireResponse::
                    get_collect_billing_details_from_wallet_connector,
                &::app::handlers::profiles::ProfileWireResponse::
                    set_collect_billing_details_from_wallet_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "always_collect_shipping_details_from_wallet_connector",
                &::app::handlers::profiles::ProfileWireResponse::
                    get_always_collect_shipping_details_from_wallet_connector,
                &::app::handlers::profiles::ProfileWireResponse::
                    set_always_collect_shipping_details_from_wallet_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "always_collect_billing_details_from_wallet_connector",
                &::app::handlers::profiles::ProfileWireResponse::
                    get_always_collect_billing_details_from_wallet_connector,
                &::app::handlers::profiles::ProfileWireResponse::
                    set_always_collect_billing_details_from_wallet_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_connector_agnostic_mit_enabled",
                &::app::handlers::profiles::ProfileWireResponse::
                    get_is_connector_agnostic_mit_enabled,
                &::app::handlers::profiles::ProfileWireResponse::
                    set_is_connector_agnostic_mit_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payout_link_config",
                &::app::handlers::profiles::ProfileWireResponse::get_payout_link_config,
                &::app::handlers::profiles::ProfileWireResponse::set_payout_link_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "outgoing_webhook_custom_http_headers",
                &::app::handlers::profiles::ProfileWireResponse::
                    get_outgoing_webhook_custom_http_headers,
                &::app::handlers::profiles::ProfileWireResponse::
                    set_outgoing_webhook_custom_http_headers,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_connector_id",
                &::app::handlers::profiles::ProfileWireResponse::get_tax_connector_id,
                &::app::handlers::profiles::ProfileWireResponse::set_tax_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_tax_connector_enabled",
                &::app::handlers::profiles::ProfileWireResponse::get_is_tax_connector_enabled,
                &::app::handlers::profiles::ProfileWireResponse::set_is_tax_connector_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_network_tokenization_enabled",
                &::app::handlers::profiles::ProfileWireResponse::
                    get_is_network_tokenization_enabled,
                &::app::handlers::profiles::ProfileWireResponse::
                    set_is_network_tokenization_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_auto_retries_enabled",
                &::app::handlers::profiles::ProfileWireResponse::get_is_auto_retries_enabled,
                &::app::handlers::profiles::ProfileWireResponse::set_is_auto_retries_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "max_auto_retries_enabled",
                &::app::handlers::profiles::ProfileWireResponse::get_max_auto_retries_enabled,
                &::app::handlers::profiles::ProfileWireResponse::set_max_auto_retries_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "always_request_extended_authorization",
                &::app::handlers::profiles::ProfileWireResponse::
                    get_always_request_extended_authorization,
                &::app::handlers::profiles::ProfileWireResponse::
                    set_always_request_extended_authorization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_click_to_pay_enabled",
                &::app::handlers::profiles::ProfileWireResponse::get_is_click_to_pay_enabled,
                &::app::handlers::profiles::ProfileWireResponse::set_is_click_to_pay_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "authentication_product_ids",
                &::app::handlers::profiles::ProfileWireResponse::get_authentication_product_ids,
                &::app::handlers::profiles::ProfileWireResponse::set_authentication_product_ids,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_testing_guard_config",
                &::app::handlers::profiles::ProfileWireResponse::get_card_testing_guard_config,
                &::app::handlers::profiles::ProfileWireResponse::set_card_testing_guard_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_clear_pan_retries_enabled",
                &::app::handlers::profiles::ProfileWireResponse::get_is_clear_pan_retries_enabled,
                &::app::handlers::profiles::ProfileWireResponse::set_is_clear_pan_retries_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "force_3ds_challenge",
                &::app::handlers::profiles::ProfileWireResponse::get_force_3ds_challenge,
                &::app::handlers::profiles::ProfileWireResponse::set_force_3ds_challenge,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_debit_routing_enabled",
                &::app::handlers::profiles::ProfileWireResponse::get_is_debit_routing_enabled,
                &::app::handlers::profiles::ProfileWireResponse::set_is_debit_routing_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_business_country",
                &::app::handlers::profiles::ProfileWireResponse::get_merchant_business_country,
                &::app::handlers::profiles::ProfileWireResponse::set_merchant_business_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_pre_network_tokenization_enabled",
                &::app::handlers::profiles::ProfileWireResponse::
                    get_is_pre_network_tokenization_enabled,
                &::app::handlers::profiles::ProfileWireResponse::
                    set_is_pre_network_tokenization_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_configs",
                &::app::handlers::profiles::ProfileWireResponse::get_acquirer_configs,
                &::app::handlers::profiles::ProfileWireResponse::set_acquirer_configs,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_config_bucket",
                &::app::handlers::profiles::ProfileWireResponse::get_acquirer_config_bucket,
                &::app::handlers::profiles::ProfileWireResponse::set_acquirer_config_bucket,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_iframe_redirection_enabled",
                &::app::handlers::profiles::ProfileWireResponse::get_is_iframe_redirection_enabled,
                &::app::handlers::profiles::ProfileWireResponse::set_is_iframe_redirection_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_category_code",
                &::app::handlers::profiles::ProfileWireResponse::get_merchant_category_code,
                &::app::handlers::profiles::ProfileWireResponse::set_merchant_category_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_country_code",
                &::app::handlers::profiles::ProfileWireResponse::get_merchant_country_code,
                &::app::handlers::profiles::ProfileWireResponse::set_merchant_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "dispute_polling_interval",
                &::app::handlers::profiles::ProfileWireResponse::get_dispute_polling_interval,
                &::app::handlers::profiles::ProfileWireResponse::set_dispute_polling_interval,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_manual_retry_enabled",
                &::app::handlers::profiles::ProfileWireResponse::get_is_manual_retry_enabled,
                &::app::handlers::profiles::ProfileWireResponse::set_is_manual_retry_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "auto_fallback_capture_method",
                &::app::handlers::profiles::ProfileWireResponse::get_auto_fallback_capture_method,
                &::app::handlers::profiles::ProfileWireResponse::set_auto_fallback_capture_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "always_enable_overcapture",
                &::app::handlers::profiles::ProfileWireResponse::get_always_enable_overcapture,
                &::app::handlers::profiles::ProfileWireResponse::set_always_enable_overcapture,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_external_vault_enabled",
                &::app::handlers::profiles::ProfileWireResponse::get_is_external_vault_enabled,
                &::app::handlers::profiles::ProfileWireResponse::set_is_external_vault_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "external_vault_connector_details",
                &::app::handlers::profiles::ProfileWireResponse::
                    get_external_vault_connector_details,
                &::app::handlers::profiles::ProfileWireResponse::
                    set_external_vault_connector_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_processor_id",
                &::app::handlers::profiles::ProfileWireResponse::get_billing_processor_id,
                &::app::handlers::profiles::ProfileWireResponse::set_billing_processor_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "surcharge_connector_details",
                &::app::handlers::profiles::ProfileWireResponse::get_surcharge_connector_details,
                &::app::handlers::profiles::ProfileWireResponse::set_surcharge_connector_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_l2_l3_enabled",
                &::app::handlers::profiles::ProfileWireResponse::get_is_l2_l3_enabled,
                &::app::handlers::profiles::ProfileWireResponse::set_is_l2_l3_enabled,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network_tokenization_credentials",
                &::app::handlers::profiles::ProfileWireResponse::
                    get_network_tokenization_credentials,
                &::app::handlers::profiles::ProfileWireResponse::
                    set_network_tokenization_credentials,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_blocking",
                &::app::handlers::profiles::ProfileWireResponse::get_payment_method_blocking,
                &::app::handlers::profiles::ProfileWireResponse::set_payment_method_blocking,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "profile_response";
    }
};
