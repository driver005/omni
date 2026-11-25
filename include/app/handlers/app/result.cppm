export module app_handlers_app:result;

import std;
import core_serde;

export namespace app::handlers::app {

// Each result is a typed DTO mirroring the flat object the runtime helpers used
// to hand-build as JSON text. FieldDesc order matches the original key order so
// serde::Ser::serialize emits the same keys in the same sequence.

class PaymentCallbackConfigResult
{
public:
    void set_partition(std::string value)
    {
        m_partition = std::move(value);
    }

    [[nodiscard]] const std::string& get_partition() const noexcept
    {
        return m_partition;
    }

    void set_op(std::string value)
    {
        m_op = std::move(value);
    }

    [[nodiscard]] const std::string& get_op() const noexcept
    {
        return m_op;
    }

    void set_mode(std::string value)
    {
        m_mode = std::move(value);
    }

    [[nodiscard]] const std::string& get_mode() const noexcept
    {
        return m_mode;
    }

    void set_issues_tokens(bool value) noexcept
    {
        m_issues_tokens = value;
    }

    [[nodiscard]] bool get_issues_tokens() const noexcept
    {
        return m_issues_tokens;
    }

    void set_allows_legacy(bool value) noexcept
    {
        m_allows_legacy = value;
    }

    [[nodiscard]] bool get_allows_legacy() const noexcept
    {
        return m_allows_legacy;
    }

    void set_config_source(std::string value)
    {
        m_config_source = std::move(value);
    }

    [[nodiscard]] const std::string& get_config_source() const noexcept
    {
        return m_config_source;
    }

private:
    std::string m_partition{};
    std::string m_op{};
    std::string m_mode{};
    bool m_issues_tokens{false};
    bool m_allows_legacy{false};
    std::string m_config_source{};
};

class CreateEmailClientResult
{
public:
    void set_partition(std::string value)
    {
        m_partition = std::move(value);
    }

    [[nodiscard]] const std::string& get_partition() const noexcept
    {
        return m_partition;
    }

    void set_op(std::string value)
    {
        m_op = std::move(value);
    }

    [[nodiscard]] const std::string& get_op() const noexcept
    {
        return m_op;
    }

    void set_client_id(std::string value)
    {
        m_client_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_client_id() const noexcept
    {
        return m_client_id;
    }

    void set_client_config(std::string value)
    {
        m_client_config = std::move(value);
    }

    [[nodiscard]] const std::string& get_client_config() const noexcept
    {
        return m_client_config;
    }

    void set_proxy_url(std::string value)
    {
        m_proxy_url = std::move(value);
    }

    [[nodiscard]] const std::string& get_proxy_url() const noexcept
    {
        return m_proxy_url;
    }

    void set_secret(std::string value)
    {
        m_secret = std::move(value);
    }

    [[nodiscard]] const std::string& get_secret() const noexcept
    {
        return m_secret;
    }

    void set_key_configured(bool value) noexcept
    {
        m_key_configured = value;
    }

    [[nodiscard]] bool get_key_configured() const noexcept
    {
        return m_key_configured;
    }

    void set_event_id(std::string value)
    {
        m_event_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_event_id() const noexcept
    {
        return m_event_id;
    }

private:
    std::string m_partition{};
    std::string m_op{};
    std::string m_client_id{};
    std::string m_client_config{};
    std::string m_proxy_url{};
    std::string m_secret{};
    bool m_key_configured{false};
    std::string m_event_id{};
};

class WithStorageResult
{
public:
    void set_partition(std::string value)
    {
        m_partition = std::move(value);
    }

    [[nodiscard]] const std::string& get_partition() const noexcept
    {
        return m_partition;
    }

    void set_op(std::string value)
    {
        m_op = std::move(value);
    }

    [[nodiscard]] const std::string& get_op() const noexcept
    {
        return m_op;
    }

    void set_session_id(std::string value)
    {
        m_session_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_session_id() const noexcept
    {
        return m_session_id;
    }

    void set_storage_impl(std::string value)
    {
        m_storage_impl = std::move(value);
    }

    [[nodiscard]] const std::string& get_storage_impl() const noexcept
    {
        return m_storage_impl;
    }

    void set_service_name(std::string value)
    {
        m_service_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_service_name() const noexcept
    {
        return m_service_name;
    }

    void set_storage_ok(bool value) noexcept
    {
        m_storage_ok = value;
    }

    [[nodiscard]] bool get_storage_ok() const noexcept
    {
        return m_storage_ok;
    }

    void set_secrets_configured(bool value) noexcept
    {
        m_secrets_configured = value;
    }

    [[nodiscard]] bool get_secrets_configured() const noexcept
    {
        return m_secrets_configured;
    }

    void set_superposition_backend(std::string value)
    {
        m_superposition_backend = std::move(value);
    }

    [[nodiscard]] const std::string& get_superposition_backend() const noexcept
    {
        return m_superposition_backend;
    }

    void set_process_tracker_id(std::int64_t value) noexcept
    {
        m_process_tracker_id = value;
    }

    [[nodiscard]] std::int64_t get_process_tracker_id() const noexcept
    {
        return m_process_tracker_id;
    }

private:
    std::string m_partition{};
    std::string m_op{};
    std::string m_session_id{};
    std::string m_storage_impl{};
    std::string m_service_name{};
    bool m_storage_ok{false};
    bool m_secrets_configured{false};
    std::string m_superposition_backend{};
    std::int64_t m_process_tracker_id{0};
};

class GetStoreInterfaceResult
{
public:
    void set_partition(std::string value)
    {
        m_partition = std::move(value);
    }

    [[nodiscard]] const std::string& get_partition() const noexcept
    {
        return m_partition;
    }

    void set_op(std::string value)
    {
        m_op = std::move(value);
    }

    [[nodiscard]] const std::string& get_op() const noexcept
    {
        return m_op;
    }

    void set_interface(std::string value)
    {
        m_interface = std::move(value);
    }

    [[nodiscard]] const std::string& get_interface() const noexcept
    {
        return m_interface;
    }

    void set_storage_impl(std::string value)
    {
        m_storage_impl = std::move(value);
    }

    [[nodiscard]] const std::string& get_storage_impl() const noexcept
    {
        return m_storage_impl;
    }

    void set_event_handler(std::string value)
    {
        m_event_handler = std::move(value);
    }

    [[nodiscard]] const std::string& get_event_handler() const noexcept
    {
        return m_event_handler;
    }

    void set_org_id(std::string value)
    {
        m_org_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_org_id() const noexcept
    {
        return m_org_id;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_tenant_known(bool value) noexcept
    {
        m_tenant_known = value;
    }

    [[nodiscard]] bool get_tenant_known() const noexcept
    {
        return m_tenant_known;
    }

    void set_key_manager_configured(bool value) noexcept
    {
        m_key_manager_configured = value;
    }

    [[nodiscard]] bool get_key_manager_configured() const noexcept
    {
        return m_key_manager_configured;
    }

    void set_mtls_cert_configured(bool value) noexcept
    {
        m_mtls_cert_configured = value;
    }

    [[nodiscard]] bool get_mtls_cert_configured() const noexcept
    {
        return m_mtls_cert_configured;
    }

    void set_mtls_ca_configured(bool value) noexcept
    {
        m_mtls_ca_configured = value;
    }

    [[nodiscard]] bool get_mtls_ca_configured() const noexcept
    {
        return m_mtls_ca_configured;
    }

    void set_event_id(std::string value)
    {
        m_event_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_event_id() const noexcept
    {
        return m_event_id;
    }

private:
    std::string m_partition{};
    std::string m_op{};
    std::string m_interface{};
    std::string m_storage_impl{};
    std::string m_event_handler{};
    std::string m_org_id{};
    std::string m_merchant_id{};
    bool m_tenant_known{false};
    bool m_key_manager_configured{false};
    bool m_mtls_cert_configured{false};
    bool m_mtls_ca_configured{false};
    std::string m_event_id{};
};

class NewResult
{
public:
    void set_partition(std::string value)
    {
        m_partition = std::move(value);
    }

    [[nodiscard]] const std::string& get_partition() const noexcept
    {
        return m_partition;
    }

    void set_op(std::string value)
    {
        m_op = std::move(value);
    }

    [[nodiscard]] const std::string& get_op() const noexcept
    {
        return m_op;
    }

    void set_session_id(std::string value)
    {
        m_session_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_session_id() const noexcept
    {
        return m_session_id;
    }

    void set_storage_impl(std::string value)
    {
        m_storage_impl = std::move(value);
    }

    [[nodiscard]] const std::string& get_storage_impl() const noexcept
    {
        return m_storage_impl;
    }

    void set_service_name(std::string value)
    {
        m_service_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_service_name() const noexcept
    {
        return m_service_name;
    }

    void set_storage_ok(bool value) noexcept
    {
        m_storage_ok = value;
    }

    [[nodiscard]] bool get_storage_ok() const noexcept
    {
        return m_storage_ok;
    }

    void set_process_tracker_id(std::int64_t value) noexcept
    {
        m_process_tracker_id = value;
    }

    [[nodiscard]] std::int64_t get_process_tracker_id() const noexcept
    {
        return m_process_tracker_id;
    }

private:
    std::string m_partition{};
    std::string m_op{};
    std::string m_session_id{};
    std::string m_storage_impl{};
    std::string m_service_name{};
    bool m_storage_ok{false};
    std::int64_t m_process_tracker_id{0};
};

} // namespace app::handlers::app

template<>
struct serde::Serializable<::app::handlers::app::PaymentCallbackConfigResult>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::app::PaymentCallbackConfigResult;
        return std::tuple{
            serde::FieldDesc<
                "partition",
                &T::get_partition,
                &T::set_partition,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"op", &T::get_op, &T::set_op, serde::FieldOptions{}>{},
            serde::FieldDesc<"mode", &T::get_mode, &T::set_mode, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issues_tokens",
                &T::get_issues_tokens,
                &T::set_issues_tokens,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "allows_legacy",
                &T::get_allows_legacy,
                &T::set_allows_legacy,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "config_source",
                &T::get_config_source,
                &T::set_config_source,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_callback_config_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::app::CreateEmailClientResult>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::app::CreateEmailClientResult;
        return std::tuple{
            serde::FieldDesc<
                "partition",
                &T::get_partition,
                &T::set_partition,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"op", &T::get_op, &T::set_op, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_id",
                &T::get_client_id,
                &T::set_client_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_config",
                &T::get_client_config,
                &T::set_client_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "proxy_url",
                &T::get_proxy_url,
                &T::set_proxy_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"secret", &T::get_secret, &T::set_secret, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "key_configured",
                &T::get_key_configured,
                &T::set_key_configured,
                serde::FieldOptions{}>{},
            serde::
                FieldDesc<"event_id", &T::get_event_id, &T::set_event_id, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "create_email_client_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::app::WithStorageResult>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::app::WithStorageResult;
        return std::tuple{
            serde::FieldDesc<
                "partition",
                &T::get_partition,
                &T::set_partition,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"op", &T::get_op, &T::set_op, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "session_id",
                &T::get_session_id,
                &T::set_session_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "storage_impl",
                &T::get_storage_impl,
                &T::set_storage_impl,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "service_name",
                &T::get_service_name,
                &T::set_service_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "storage_ok",
                &T::get_storage_ok,
                &T::set_storage_ok,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "secrets_configured",
                &T::get_secrets_configured,
                &T::set_secrets_configured,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "superposition_backend",
                &T::get_superposition_backend,
                &T::set_superposition_backend,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "process_tracker_id",
                &T::get_process_tracker_id,
                &T::set_process_tracker_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "with_storage_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::app::GetStoreInterfaceResult>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::app::GetStoreInterfaceResult;
        return std::tuple{
            serde::FieldDesc<
                "partition",
                &T::get_partition,
                &T::set_partition,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"op", &T::get_op, &T::set_op, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "interface",
                &T::get_interface,
                &T::set_interface,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "storage_impl",
                &T::get_storage_impl,
                &T::set_storage_impl,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "event_handler",
                &T::get_event_handler,
                &T::set_event_handler,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"org_id", &T::get_org_id, &T::set_org_id, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &T::get_merchant_id,
                &T::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tenant_known",
                &T::get_tenant_known,
                &T::set_tenant_known,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "key_manager_configured",
                &T::get_key_manager_configured,
                &T::set_key_manager_configured,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mtls_cert_configured",
                &T::get_mtls_cert_configured,
                &T::set_mtls_cert_configured,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mtls_ca_configured",
                &T::get_mtls_ca_configured,
                &T::set_mtls_ca_configured,
                serde::FieldOptions{}>{},
            serde::
                FieldDesc<"event_id", &T::get_event_id, &T::set_event_id, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "get_store_interface_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::app::NewResult>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::app::NewResult;
        return std::tuple{
            serde::FieldDesc<
                "partition",
                &T::get_partition,
                &T::set_partition,
                serde::FieldOptions{}>{},
            serde::FieldDesc<"op", &T::get_op, &T::set_op, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "session_id",
                &T::get_session_id,
                &T::set_session_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "storage_impl",
                &T::get_storage_impl,
                &T::set_storage_impl,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "service_name",
                &T::get_service_name,
                &T::set_service_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "storage_ok",
                &T::get_storage_ok,
                &T::set_storage_ok,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "process_tracker_id",
                &T::get_process_tracker_id,
                &T::set_process_tracker_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "new_result";
    }
};
