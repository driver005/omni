export module app_handlers_app:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_payment_callback_config;
import :runtime_create_email_client;
import :runtime_with_storage;
import :runtime_get_store_interface;
import :runtime_new;

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

    [[nodiscard]] std::expected<PaymentCallbackConfigResult, core::error::AppError>
    payment_callback_config(const PaymentCallbackConfigInput& input) const
    {
        return m_payment_callback_config.payment_callback_config(input);
    }

    [[nodiscard]] std::expected<CreateEmailClientResult, core::error::AppError>
    create_email_client(const CreateEmailClientInput& input) const
    {
        return m_create_email_client.create_email_client(input);
    }

    [[nodiscard]] std::expected<WithStorageResult, core::error::AppError>
    with_storage(const WithStorageInput& input) const
    {
        return m_with_storage.with_storage(input);
    }

    [[nodiscard]] std::expected<GetStoreInterfaceResult, core::error::AppError>
    get_store_interface(const GetStoreInterfaceInput& input) const
    {
        return m_get_store_interface.get_store_interface(input);
    }

    [[nodiscard]] std::expected<NewResult, core::error::AppError>
    app_new(const NewInput& input) const
    {
        return m_new.app_new(input);
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
