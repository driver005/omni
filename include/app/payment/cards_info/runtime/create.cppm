export module app_payment:cards_info_runtime_create;

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

class CreateCardsInfoRuntime
{
public:
    explicit CreateCardsInfoRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: create_cards_info ─────────────────────────
    // core/cards_info.rs:56-67 create_card_info, wrapped by
    // api::server_wrap(Flow::CardsInfoCreate, ..., LockAction::NotApplicable).

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::cards_info::CardInfoResponse>
        create(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = CreateCardsInfoData{dto, ctx};
        auto repository = m_context.repository();
        auto created = repository.create_card_info(m_support.to_store_record(input.request));
        if (!created) {
            m_context.metrics().record_flow("CardsInfoCreate");
            return std::unexpected{m_support.generic_duplicate_error(
                "CardInfo with given key already exists in our records"
            )};
        }
        auto response = m_support.build_response(input.request);
        m_support.cache_response(created->get_iin(), response);

        m_context.metrics().record_flow("CardsInfoCreate");
        m_context.metrics().counter("CARDS_INFO_CREATED").increment();
        return ::app::model::cards_info::CardInfoResponse{
            m_support.response_from_store_enriched(*created, std::move(response))
        };
    }

private:
    ::app::context::AppContext& m_context;
    CardsInfoRuntimeSupport m_support;
};

} // namespace app::handlers::cards_info
