export module app_store:app_runtime;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_context;

import :app_result;
import :app_runtime_common;
import :app_runtime_payment_callback_config;
import :app_runtime_create_email_client;
import :app_runtime_with_storage;
import :app_runtime_get_store_interface;
import :app_runtime_new;

export namespace app::handlers::app {

class AppRuntime
{
public:
    explicit AppRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_payment_callback_config{context},
        m_create_email_client{context},
        m_with_storage{context},
        m_get_store_interface{context},
        m_new{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    payment_callback_config(const core::behavior::CallContext& ctx) const
    {
        return m_payment_callback_config.payment_callback_config(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    create_email_client(const serde::Value& payload, const core::behavior::CallContext& ctx) const
    {
        return m_create_email_client.create_email_client(payload, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    with_storage(const core::behavior::CallContext& ctx) const
    {
        return m_with_storage.with_storage(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_store_interface(const core::behavior::CallContext& ctx) const
    {
        return m_get_store_interface.get_store_interface(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    app_new(const core::behavior::CallContext& ctx) const
    {
        return m_new.app_new(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentCallbackConfigRuntime m_payment_callback_config;
    CreateEmailClientRuntime m_create_email_client;
    WithStorageRuntime m_with_storage;
    GetStoreInterfaceRuntime m_get_store_interface;
    NewRuntime m_new;
};

} // namespace app::handlers::app
