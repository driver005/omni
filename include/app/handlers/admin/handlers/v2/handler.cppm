export module app_handlers_admin:handlers_v2;

import std;
import interfaces;
import core_router;
import utils_openapi;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_v2_organizations;
import :handlers_v2_merchant_accounts;
import :handlers_v2_kv;
import :handlers_v2_connector_accounts;
import :handlers_v2_connectors;

export namespace app::handlers::admin {

class AdminHandlersV2
{
public:
    AdminHandlersV2(
        app::context::AppContext& context,
        AdminRuntime& runtime,
        AdminInputSupport& input_support
    ) noexcept :
        m_organizations{context, runtime, input_support},
        m_merchant_accounts{context, runtime, input_support},
        m_kv{context, runtime, input_support},
        m_connector_accounts{context, runtime, input_support},
        m_connectors{context, runtime, input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        // Original registration order is preserved across family fragments.
        m_organizations.install(router);
        m_merchant_accounts.install(router);
        m_kv.install(router);
        m_connector_accounts.install(router);
        m_connectors.install(router);
    }

private:
    AdminHandlersV2Organizations m_organizations;
    AdminHandlersV2MerchantAccounts m_merchant_accounts;
    AdminHandlersV2Kv m_kv;
    AdminHandlersV2ConnectorAccounts m_connector_accounts;
    AdminHandlersV2Connectors m_connectors;
};

} // namespace app::handlers::admin
