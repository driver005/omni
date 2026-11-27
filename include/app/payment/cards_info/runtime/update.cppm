export module app_payment:cards_info_runtime_update;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_model;
import app_store;

import :cards_info_input;
import :cards_info_result;
import :cards_info_runtime_common;

export namespace app::handlers::cards_info {

class UpdateCardsInfoRuntime
{
public:
    explicit UpdateCardsInfoRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: update_cards_info ─────────────────────────
    // core/cards_info.rs:70-125 update_card_info, wrapped by
    // api::server_wrap(Flow::CardsInfoUpdate, ..., LockAction::NotApplicable).

    [[nodiscard]] std::expected<::app::model::cards_info::CardInfoResponse, core::error::AppError>
    update(const UpdateCardsInfoInput& input) const
    {
        auto repository = m_context.repository();
        auto stored = repository.find_card_info_by_string(input.request.get_card_iin());
        if (!stored) {
            m_context.metrics().record_flow("CardsInfoUpdate");
            return std::unexpected{m_support.generic_not_found_error(
                "Card info with given key does not exist in our records"
            )};
        }
        m_support.apply_update(*stored, input.request); // last_updated = now()
        if (!repository.update_card_info(*stored)) {
            m_context.metrics().record_flow("CardsInfoUpdate");
            return std::unexpected{
                m_support.internal_server_error("Failed while updating card info")
            };
        }
        auto response = m_support.build_response(input.request);
        m_support.cache_response(stored->get_iin(), response);

        m_context.metrics().record_flow("CardsInfoUpdate");
        m_context.metrics().counter("CARDS_INFO_UPDATED").increment();
        return ::app::model::cards_info::CardInfoResponse{
            m_support.response_from_store_enriched(*stored, std::move(response))
        };
    }

private:
    ::app::context::AppContext& m_context;
    CardsInfoRuntimeSupport m_support;
};

} // namespace app::handlers::cards_info
