export module app_handlers_payout_link:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_render_payout_link;

export namespace app::handlers::payout_link {

class PayoutLinkRuntime
{
public:
    explicit PayoutLinkRuntime(app::context::AppContext& context) noexcept :
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
    app::context::AppContext& m_context;
    PayoutLinkRenderRuntime m_render;
};

} // namespace app::handlers::payout_link
