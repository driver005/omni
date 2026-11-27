export module app_payment:payouts_handlers_v2;

import std;
import interfaces;
import core_router;
import utils_openapi;
import app_context;

import :payouts_runtime;
import :payouts_input;
import :payouts_handlers_v2_list_filter;

export namespace app::handlers::payouts {

class PayoutsHandlersV2
{
public:
    void bind_request_support(PayoutsRequestSupport& support) noexcept
    {
        m_request_support = &support;
        m_list_filter.bind_request_support(support);
    }

    PayoutsRequestSupport* m_request_support{nullptr};

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
