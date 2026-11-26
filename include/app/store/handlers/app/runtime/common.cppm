export module app_store:app_runtime_common;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_context_core;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

export namespace app::handlers::app {

/**
 * @brief Shared runtime helpers for the app sub-runtimes.
 *
 * No HTTP types live here; every helper returns a value or an AppError.
 */
class AppRuntimeSupport
{
public:
    explicit AppRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    [[nodiscard]] std::optional<std::string>
    field_string(const serde::Value& value, std::string_view name) const
    {
        const auto* object = std::get_if<serde::Value::Object>(&value.get());
        if (object == nullptr) {
            return std::nullopt;
        }
        const auto field = object->get(std::string{name});
        if (!field) {
            return std::nullopt;
        }
        if (const auto* text = std::get_if<std::string>(&field.value().get())) {
            return *text;
        }
        return std::nullopt;
    }

    /** @brief Encodes a typed JSON document for persisted event/tracker payloads. */
    [[nodiscard]] std::string serialize_document(serde::Value document) const
    {
        const auto bytes = serde::Ser::serialize("application/json", document);
        return std::string{reinterpret_cast<const char*>(bytes.data()), bytes.size()};
    }

    [[nodiscard]] bool known_storage_impl(std::string_view value) const noexcept
    {
        return value == "postgresql" || value == "postgresql_test" || value == "mock";
    }

    // PaymentCallbackConfig is a config entry (stored as JSON), not one of the typed domain
    // entities. ConfigStore is the persisted source, repository config/routing-config are the
    // compatibility mirrors, and the cache stands in for the session-local OnceCell.
    [[nodiscard]] std::optional<std::string> load_payment_callback_config(std::string& source) const
    {
        constexpr std::string_view key{"payment_callback"};
        constexpr std::string_view cache_key{"app.config.payment_callback"};
        const auto cached = m_context.cache().get(std::string{cache_key});
        if (cached.hit) {
            source = "session_cache";
            return cached.value_or<std::string>(std::string{});
        }
        try {
            const auto config_store_value =
                m_context.config_store().get_value("payment_callback", "config");
            if (config_store_value && !config_store_value->empty()) {
                m_context.cache().put(
                    std::string{cache_key},
                    core::cache::to_cache_value(*config_store_value)
                );
                source = "config_store";
                return config_store_value;
            }
            const auto repo_value = m_context.repository().get_config(key);
            if (repo_value && !repo_value->empty()) {
                m_context.cache().put(
                    std::string{cache_key},
                    core::cache::to_cache_value(*repo_value)
                );
                source = "repository_config";
                return repo_value;
            }
            const auto record = m_context.repository().find_routing_config_by_string(key);
            if (record && !record->get_config_json().empty()) {
                const auto text = record->get_config_json();
                m_context.cache().put(std::string{cache_key}, core::cache::to_cache_value(text));
                source = "routing_config";
                return text;
            }
        } catch (...) {
            return std::nullopt;
        }
        return std::nullopt;
    }

    [[nodiscard]] bool storage_available() const
    {
        try {
            (void)m_context.repository().list_organizations();
            return true;
        } catch (...) {
            return false;
        }
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::app
