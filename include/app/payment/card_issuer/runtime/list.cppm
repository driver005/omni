export module app_payment:card_issuer_runtime_list;
import core_behavior;

import std;
import core_error;
import app_context;
import app_dto;
import app_metrics;
import app_store;

import :card_issuer_input;
import :card_issuer_result;
import :card_issuer_runtime_common;

export namespace app::handlers::card_issuer {

class CardIssuerListRuntime
{
public:
    explicit CardIssuerListRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // core/card_issuer.rs:84-119 `list_card_issuers`.
    [[nodiscard]] core::behavior::Outcome<::app::dto::card_issuer::CardIssuerListResponse>
    list(const core::behavior::CallContext& ctx) const
    {
        (void)ctx;
        const std::size_t limit = m_support.configured_list_limit();

        auto repository = m_context.repository();
        auto records = repository.list_card_issuers();

        // core/card_issuer.rs:103-106 warn!("Card issuer list hit the configured limit").
        // No tracing sink is injected locally; a named counter is the deterministic
        // in-process substitute.
        if (records.size() >= limit) {
            m_context.logger().counter("CARD_ISSUER_LIST_LIMIT_REACHED").increment();
        }

        std::vector<::app::dto::card_issuer::CardIssuerResponse> issuers;
        issuers.reserve(std::min(limit, records.size()));
        for (const auto& record: records) {
            // Upstream asks the store for at most `limit` rows; the local store
            // returns all rows, so the same bound is applied here.
            if (issuers.size() >= limit) {
                break;
            }
            // ForeignTryInto<CardIssuerListItem> (types/transformers.rs:3018-3032):
            // a bad stored issuer_name maps to InvalidDataValue { "issuer_name" },
            // which upstream renders as 400 "Invalid value provided: issuer_name".
            if (m_support.validate_issuer_name(record.get_name())) {
                return std::unexpected{m_support.make_error(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "Invalid value provided: issuer_name"
                )};
            }
            issuers.push_back(m_support.response_from(record));
        }

        ::app::dto::card_issuer::CardIssuerListResponse response;
        response.set_issuers(std::move(issuers));

        // `#[instrument(skip_all, fields(flow = Flow::ListCardIssuers))]` substitute.
        m_context.logger().record_flow("ListCardIssuers");
        m_context.logger().counter("CARD_ISSUERS_LISTED").increment();

        return ::app::dto::card_issuer::CardIssuerListResponse{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    CardIssuerRuntimeSupport m_support;
};

} // namespace app::handlers::card_issuer
