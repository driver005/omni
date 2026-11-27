export module app_store:app_result;

import std;

export namespace app::handlers::app {

// Exact wire field lists captured from the retired *Result Serializable field
// tuples. Each runtime helper builds a serde::Value::Object in this order, so
// the encoded body stays byte-identical to the old typed serialization.

[[nodiscard]] inline constexpr auto payment_callback_config_wire_fields()
{
    return std::array<std::string_view, 6>{
        "partition",
        "op",
        "mode",
        "issues_tokens",
        "allows_legacy",
        "config_source"
    };
}

[[nodiscard]] inline constexpr auto create_email_client_wire_fields()
{
    return std::array<std::string_view, 8>{
        "partition",
        "op",
        "client_id",
        "client_config",
        "proxy_url",
        "secret",
        "key_configured",
        "event_id"
    };
}

[[nodiscard]] inline constexpr auto with_storage_wire_fields()
{
    return std::array<std::string_view, 9>{
        "partition",
        "op",
        "session_id",
        "storage_impl",
        "service_name",
        "storage_ok",
        "secrets_configured",
        "superposition_backend",
        "process_tracker_id"
    };
}

[[nodiscard]] inline constexpr auto get_store_interface_wire_fields()
{
    return std::array<std::string_view, 12>{
        "partition",
        "op",
        "interface",
        "storage_impl",
        "event_handler",
        "org_id",
        "merchant_id",
        "tenant_known",
        "key_manager_configured",
        "mtls_cert_configured",
        "mtls_ca_configured",
        "event_id"
    };
}

[[nodiscard]] inline constexpr auto new_wire_fields()
{
    return std::array<std::string_view, 7>{
        "partition",
        "op",
        "session_id",
        "storage_impl",
        "service_name",
        "storage_ok",
        "process_tracker_id"
    };
}

} // namespace app::handlers::app
