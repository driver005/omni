export module app_user:admin_result;

import std;
import core_serde;
import app_dto;
import app_model;

export namespace app::handlers::admin {

// Wire field lists captured from the former Result Serializable specializations.
// Each Result reused the full FieldDesc tuple of its underlying model, so these
// lists hold the exact declared field names in Serializable<T>::fields() order.

inline constexpr std::array<std::string_view, 7> kOrganizationResponseWireFields{
    "organization_id",
    "organization_name",
    "organization_details",
    "metadata",
    "modified_at",
    "created_at",
    "organization_type",
};

inline constexpr std::array<std::string_view, 3> kConvertOrganizationToPlatformResponseWireFields{
    "organization_id",
    "organization_type",
    "platform_merchant_id",
};

inline constexpr std::array<std::string_view, 25> kMerchantAccountResponseWireFields{
    "merchant_id",
    "merchant_name",
    "return_url",
    "enable_payment_response_hash",
    "payment_response_hash_key",
    "redirect_to_merchant_with_http_post",
    "merchant_details",
    "webhook_details",
    "routing_algorithm",
    "payout_routing_algorithm",
    "sub_merchants_enabled",
    "parent_merchant_id",
    "publishable_key",
    "metadata",
    "locker_id",
    "primary_business_details",
    "frm_routing_algorithm",
    "organization_id",
    "is_recon_enabled",
    "default_profile",
    "recon_status",
    "pm_collect_link_config",
    "product_type",
    "merchant_account_type",
    "network_tokenization_credentials",
};

inline constexpr std::array<std::string_view, 2> kMerchantAccountDeleteResponseWireFields{
    "merchant_id",
    "deleted",
};

inline constexpr std::array<std::string_view, 17> kMerchantConnectorResponseWireFields{
    "connector_type",
    "connector_name",
    "connector_label",
    "id",
    "profile_id",
    "connector_account_details",
    "payment_methods_enabled",
    "connector_webhook_details",
    "metadata",
    "disabled",
    "frm_configs",
    "applepay_verified_domains",
    "pm_auth_config",
    "status",
    "additional_merchant_data",
    "connector_wallets_details",
    "feature_metadata",
};

inline constexpr std::array<std::string_view, 15> kMerchantConnectorListResponseWireFields{
    "connector_type",
    "connector_name",
    "connector_label",
    "merchant_connector_id",
    "profile_id",
    "payment_methods_enabled",
    "test_mode",
    "disabled",
    "frm_configs",
    "business_country",
    "business_label",
    "business_sub_label",
    "applepay_verified_domains",
    "pm_auth_config",
    "status",
};

inline constexpr std::array<std::string_view, 3> kMerchantConnectorDeleteResponseWireFields{
    "merchant_id",
    "merchant_connector_id",
    "deleted",
};

inline constexpr std::array<std::string_view, 2> kToggleKVResponseWireFields{
    "merchant_id",
    "kv_enabled",
};

inline constexpr std::array<std::string_view, 2> kToggleAllKVResponseWireFields{
    "total_updated",
    "kv_enabled",
};

inline constexpr std::array<std::string_view, 1> kTransferKeyResponseWireFields{
    "total_transferred",
};

inline constexpr std::array<std::string_view, 13> kWebhookDetailsResponseWireFields{
    "webhook_version",
    "webhook_username",
    "webhook_password",
    "webhook_url",
    "payment_created_enabled",
    "payment_succeeded_enabled",
    "payment_failed_enabled",
    "payment_statuses_enabled",
    "refund_statuses_enabled",
    "payout_statuses_enabled",
    "dispute_statuses_enabled",
    "mandate_statuses_enabled",
    "invoice_statuses_enabled",
};

} // namespace app::handlers::admin
