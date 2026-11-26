export module app_payment:payout_link_runtime;

import std;
import core_error;
import app_context;

import :payout_link_input;
import :payout_link_result;
import :payout_link_runtime_common;
import :payout_link_runtime_render_payout_link;

export namespace app::handlers::payout_link {

class PayoutLinkRuntime
{
public:
    explicit PayoutLinkRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_render{context}
    {
    }

    [[nodiscard]] std::expected<PayoutLinkRenderResult, core::error::AppError>
    render_payout_link(const PayoutLinkRenderInput& input) const
    {
        return m_render.render(input);
    }

private:
    ::app::context::AppContext& m_context;
    PayoutLinkRenderRuntime m_render;
};

} // namespace app::handlers::payout_link
