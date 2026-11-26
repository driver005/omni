export module app_user:admin_handlers_v1;

import std;
import interfaces;
import core_router;
import utils_openapi;
import app_context;

import :admin_input;
import :admin_result;
import :admin_runtime;
import :admin_handlers_v1_organizations;
import :admin_handlers_v1_merchant_accounts;
import :admin_handlers_v1_kv;
import :admin_handlers_v1_connector_accounts;
import :admin_handlers_v1_connectors;
import :admin_handlers_v1_webhooks;

export namespace app::handlers::admin {

class AdminHandlersV1
{
public:
    AdminHandlersV1(
        ::app::context::AppContext& context,
        AdminRuntime& runtime,
        AdminInputSupport& input_support
    ) noexcept :
        m_organizations{context, runtime, input_support},
        m_merchant_accounts{context, runtime, input_support},
        m_kv{context, runtime, input_support},
        m_connector_accounts{context, runtime, input_support},
        m_connectors{context, runtime, input_support},
        m_webhooks{context, runtime, input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        // Original registration order is preserved across family fragments.
        m_organizations.install(router);
        m_merchant_accounts.install(router);
        m_kv.install(router);
        m_merchant_accounts.install_middle(router);
        m_kv.install_tail(router);
        m_merchant_accounts.install_tail(router);
        m_connector_accounts.install(router);
        m_connector_accounts.install_middle(router);
        m_connectors.install(router);
        m_connector_accounts.install_tail(router);
        m_webhooks.install(router);
        m_connectors.install_profile(router);
    }

private:
    AdminHandlersV1Organizations m_organizations;
    AdminHandlersV1MerchantAccounts m_merchant_accounts;
    AdminHandlersV1Kv m_kv;
    AdminHandlersV1ConnectorAccounts m_connector_accounts;
    AdminHandlersV1Connectors m_connectors;
    AdminHandlersV1Webhooks m_webhooks;
};

} // namespace app::handlers::admin
