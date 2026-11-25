export module app_handlers_cards_info:runtime_card_iin_info;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_model;
import app_store;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::cards_info {

class CardIinInfoRuntime
{
public:
    explicit CardIinInfoRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: card_iin_info ─────────────────────────
    // core/cards_info.rs:32-53 retrieve_card_info, wrapped by
    // api::server_wrap(Flow::CardsInfo, ..., LockAction::NotApplicable).

    [[nodiscard]] std::expected<CardIinInfoResult, core::error::AppError>
    card_iin_info(const CardIinInfoInput& input) const
    {
        // core/cards_info.rs:32-53 retrieve_card_info.
        const auto& payload = input.payload;
        if (const auto invalid = m_support.verify_iin_length(payload.get_card_iin())) {
            m_context.metrics().record_flow("CardsInfo");
            return std::unexpected{*invalid};
        }
        if (const auto invalid = m_support.verify_payment_intent_time_and_client_secret(
                payload.get_client_secret()
            )) {
            m_context.metrics().record_flow("CardsInfo");
            return std::unexpected{*invalid};
        }
        const auto stored = m_context.repository().find_card_info_by_string(payload.get_card_iin());
        if (!stored) {
            m_context.metrics().record_flow("CardsInfo");
            return std::unexpected{m_support.invalid_card_iin()};
        }
        auto fallback =
            m_support.cached_response(payload.get_card_iin()).value_or(CardInfoResponse{});
        auto response = m_support.response_from_store_enriched(*stored, std::move(fallback));

        // api::server_wrap records the flow before mapping the outcome to the wire.
        m_context.metrics().record_flow("CardsInfo");
        m_context.metrics().counter("CARDS_INFO_RETRIEVED").increment();
        return CardIinInfoResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    CardsInfoRuntimeSupport m_support;
};

} // namespace app::handlers::cards_info
