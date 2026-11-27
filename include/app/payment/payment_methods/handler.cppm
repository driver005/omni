export module app_payment:payment_methods_handler;

import std;
import interfaces;
import core_router;
import app_context;

import :payment_methods_input;
import :context;
import :payment_methods_result;
import :payment_methods_runtime;
import :payment_methods_handlers_crud;
import :payment_methods_handlers_migrate;
import :payment_methods_handlers_payment_method_sessions;
import :payment_methods_handlers_tokenize;
import :payment_methods_handlers_network_token;
import :payment_methods_handlers_client_list;
import :payment_methods_handlers_olap_support;

export namespace app::handlers::payment_methods {

class PaymentMethodsRouteHandlers
{
public:
    explicit PaymentMethodsRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_crud{context, m_runtime},
        m_migrate{context, m_runtime},
        m_sessions{context, m_runtime},
        m_tokenize{context, m_runtime},
        m_network{context, m_runtime},
        m_client_list{context, m_runtime},
        m_olap_support{context, m_runtime},
        m_input_support{context},
        m_policy{context, m_input_support},
        m_request_support{m_policy}
    {
        m_crud.bind_request_support(m_request_support);
        m_migrate.bind_request_support(m_request_support);
        m_sessions.bind_request_support(m_request_support);
        m_tokenize.bind_request_support(m_request_support);
        m_network.bind_request_support(m_request_support);
        m_client_list.bind_request_support(m_request_support);
        m_olap_support.bind_request_support(m_request_support);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_crud.install(router);
        m_client_list.install(router);
        m_migrate.install(router);
        m_tokenize.install(router);
        m_olap_support.install(router);
        m_network.install(router);
        m_sessions.install(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntime m_runtime;
    CrudHandlers m_crud;
    MigrateHandlers m_migrate;
    PaymentMethodSessionsHandlers m_sessions;
    TokenizeHandlers m_tokenize;
    NetworkTokenHandlers m_network;
    ClientListHandlers m_client_list;
    OlapSupportHandlers m_olap_support;
    PaymentMethodsInputSupport m_input_support;
    PaymentMethodsPolicy m_policy;
    PaymentMethodsRequestSupport m_request_support;
};

} // namespace app::handlers::payment_methods
