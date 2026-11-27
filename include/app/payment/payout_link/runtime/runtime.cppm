export module app_payment:payout_link_runtime;

import std;
import core_serde;
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

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    render_payout_link(const core::behavior::CallContext& ctx) const
    {
        return m_render.render(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    PayoutLinkRenderRuntime m_render;
};

} // namespace app::handlers::payout_link
