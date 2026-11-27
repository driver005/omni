export module app_auth:authentication_result;

import std;

export namespace app::handlers::authentication {

/**
 * @brief Exact wire field lists captured from the retired Result classes' Serializable
 * specializations. Each one mirrors the corresponding DTO's FieldDesc tuple in declaration
 * order, so Serializer<T>::serialize(fields) emits the same bytes the old core did.
 */
[[nodiscard]] inline constexpr auto create_wire_fields()
{
    return std::array<std::string_view, 17>{
        "authentication_id",
        "merchant_id",
        "status",
        "client_secret",
        "amount",
        "currency",
        "authentication_connector",
        "force_3ds_challenge",
        "return_url",
        "created_at",
        "error_code",
        "error_message",
        "profile_id",
        "psd2_sca_exemption_type",
        "acquirer_details",
        "profile_acquirer_id",
        "customer_details"
    };
}

[[nodiscard]] inline constexpr auto authenticate_wire_fields()
{
    return std::array<std::string_view, 17>{
        "trans_status",
        "acs_url",
        "challenge_request",
        "acs_reference_number",
        "acs_trans_id",
        "three_ds_server_transaction_id",
        "acs_signed_content",
        "three_ds_requestor_url",
        "three_ds_requestor_app_url",
        "error_message",
        "error_code",
        "authentication_value",
        "status",
        "authentication_connector",
        "authentication_id",
        "eci",
        "acquirer_details"
    };
}

[[nodiscard]] inline constexpr auto sync_wire_fields()
{
    return std::array<std::string_view, 40>{
        "authentication_id",
        "merchant_id",
        "status",
        "client_secret",
        "amount",
        "currency",
        "authentication_connector",
        "force_3ds_challenge",
        "return_url",
        "created_at",
        "profile_id",
        "psd2_sca_exemption_type",
        "acquirer_details",
        "threeds_server_transaction_id",
        "maximum_supported_3ds_version",
        "connector_authentication_id",
        "three_ds_method_data",
        "three_ds_method_url",
        "message_version",
        "connector_metadata",
        "directory_server_id",
        "payment_method_data",
        "vault_token_data",
        "authentication_details",
        "billing",
        "shipping",
        "browser_information",
        "email",
        "trans_status",
        "acs_url",
        "challenge_request",
        "acs_reference_number",
        "acs_trans_id",
        "acs_signed_content",
        "three_ds_requestor_url",
        "three_ds_requestor_app_url",
        "eci",
        "error_message",
        "error_code",
        "profile_acquirer_id"
    };
}

// Eligibility, eligibility-check, retrieve-eligibility-check, session-token and post-sync
// responses are externally-tagged / hand-built wire documents. They travel as serde::Value
// built in the runtime and are encoded with Ser::serialize; no Result wrapper is involved.

} // namespace app::handlers::authentication
