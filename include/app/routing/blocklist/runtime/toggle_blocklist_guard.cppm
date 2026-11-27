export module app_routing:blocklist_runtime_toggle_blocklist_guard;

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
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;
import core_behavior;

import :blocklist_result;
import :blocklist_runtime_common;

export namespace app::handlers::blocklist {

class ToggleBlocklistGuardRuntime
{
public:
    explicit ToggleBlocklistGuardRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // utils.rs:76-118. Merchant-wide, no profile.
    [[nodiscard]] core::behavior::Outcome<::app::model::blocklist::ToggleBlocklistResponse>
    toggle_blocklist_guard(const ::core::behavior::CallContext& ctx) const
    {
        const bool status = m_support.query_value(ctx, "status").value_or("false") == "true";
        // common_utils/id_type/merchant.rs:126-128: guard_blocklist_for_{merchant}.
        const auto key = "guard_blocklist_for_" + ctx.merchant_id;
        m_context.config_store().put("configs", key, status ? "true" : "false");

        ::app::model::blocklist::ToggleBlocklistResponse response;
        response.set_blocklist_guard_status(status ? "enabled" : "disabled");
        m_support.emit("blocklist.guard.toggled", response);
        m_support.track("toggle_blocklist_guard");
        return response;
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistRuntimeSupport m_support;
};

} // namespace app::handlers::blocklist
