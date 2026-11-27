export module app_store:app_runtime_payment_callback_config;

import std;
import core_behavior;
import interfaces;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_context_core;
import app_crypto;
import app_metrics;
import app_context_core;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :app_result;
import :app_runtime_common;

export namespace app::handlers::app {

class PaymentCallbackConfigRuntime
{
public:
    explicit PaymentCallbackConfigRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream SessionState::payment_callback_config() is an internal OnceCell load
    // (app.rs:207-212) with no HTTP auth. The registry row is NO_AUTH.
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    payment_callback_config(const core::behavior::CallContext& ctx) const
    {
        (void)ctx;

        // Load the runtime rollout policy stored under the "payment_callback" config key.
        // ConfigStore (superposition-backed) is consulted first, then the repository config
        // and routing-config mirrors; the cache is only the session-local OnceCell stand-in.
        std::string config_source;
        const auto stored = m_support.load_payment_callback_config(config_source);
        if (!stored) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "payment_callback config is missing"
            )};
        }
        auto parsed = serde::Ser::decode_generic("application/json", *stored);
        if (!parsed) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid payment_callback config"
            )};
        }

        const auto mode = m_support.field_string(*parsed, "mode").value_or("");
        if (mode != "legacy" && mode != "migration" && mode != "enforced") {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "payment_callback mode must be legacy, migration or enforced"
            )};
        }
        // Mirrors PaymentCallbackMode::issues_tokens()/allows_legacy().
        const bool issues_tokens = mode != "legacy";
        const bool allows_legacy = mode != "enforced";

        m_context.logger().record_flow("PaymentCallbackConfigLoad");
        m_context.logger().counter("PAYMENT_CALLBACK_CONFIG_LOADED").increment();

        serde::Value::Object document{};
        document.insert(
            std::string{payment_callback_config_wire_fields()[0]},
            serde::Value{std::string{"app"}}
        );
        document.insert(
            std::string{payment_callback_config_wire_fields()[1]},
            serde::Value{std::string{"payment_callback_config"}}
        );
        document.insert(std::string{payment_callback_config_wire_fields()[2]}, serde::Value{mode});
        document.insert(
            std::string{payment_callback_config_wire_fields()[3]},
            serde::Value{issues_tokens}
        );
        document.insert(
            std::string{payment_callback_config_wire_fields()[4]},
            serde::Value{allows_legacy}
        );
        document.insert(
            std::string{payment_callback_config_wire_fields()[5]},
            serde::Value{config_source}
        );
        return serde::Value{std::move(document)};
    }

private:
    ::app::context::AppContext& m_context;
    AppRuntimeSupport m_support;
};

} // namespace app::handlers::app
