export module app_handlers_payment_methods:handler;

import std;
import interfaces;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_crud;
import :handlers_migrate;
import :handlers_payment_method_sessions;
import :handlers_tokenize;
import :handlers_network_token;
import :handlers_client_list;
import :handlers_olap_support;

export namespace app::handlers::payment_methods {

class PaymentMethodsRouteHandlers
{
public:
    explicit PaymentMethodsRouteHandlers(app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_crud{context, m_runtime},
        m_migrate{context, m_runtime},
        m_sessions{context, m_runtime},
        m_tokenize{context, m_runtime},
        m_network{context, m_runtime},
        m_client_list{context, m_runtime},
        m_olap_support{context, m_runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
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
    app::context::AppContext& m_context;
    PaymentMethodsRuntime m_runtime;
    CrudHandlers m_crud;
    MigrateHandlers m_migrate;
    PaymentMethodSessionsHandlers m_sessions;
    TokenizeHandlers m_tokenize;
    NetworkTokenHandlers m_network;
    ClientListHandlers m_client_list;
    OlapSupportHandlers m_olap_support;
};

} // namespace app::handlers::payment_methods
