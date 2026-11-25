export module app_handlers_payouts:handlers_v2;

import std;
import interfaces;
import core_router;
import utils_openapi;
import app_context;

import :runtime;
import :input;
import :handlers_v2_list_filter;

export namespace app::handlers::payouts {

class PayoutsHandlersV2
{
public:
    explicit PayoutsHandlersV2(PayoutsRuntime& runtime, PayoutsInputSupport& input_support) noexcept
        :
        m_list_filter{runtime, input_support}
    {
    }

    [[nodiscard]] utils::openapi::ApiRouter
    install(utils::openapi::ApiRouter chain, core::router::RouterContext<>& router)
    {
        return m_list_filter.install(std::move(chain), router);
    }

private:
    PayoutsListFilterHandlersV2 m_list_filter;
};

} // namespace app::handlers::payouts
