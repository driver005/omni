export module app_payment:mandates_runtime_retrieve_mandates_list;

import std;
import core_serde;
import core_error;
import app_context;
import app_store;
import app_model;

import :mandates_input;
import :mandates_result;
import :mandates_runtime_common;

export namespace app::handlers::mandates {

class RetrieveMandatesListRuntime
{
public:
    explicit RetrieveMandatesListRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: retrieve_mandates_list ─────────────────────────
    // core/mandate.rs:403-426.
    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<std::vector<::app::model::mandates::MandateResponse>>
        retrieve_mandates_list(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = RetrieveMandatesListData{dto, ctx};
        m_support.record_flow("MandatesList"); // LockAction::NotApplicable (routes/mandates.rs:98)

        // find_mandates_by_merchant_id(merchant_id, constraints) (core/mandate.rs:409-415).
        // MandateQuery applies merchant/status/limit/offset at the database boundary;
        // connector + created_time are applied below (the local query object has no column
        // for them).
        auto db_query = ::app::store::MandateQuery{};
        if (input.merchant_filter.has_value() && !input.merchant_filter->empty()) {
            db_query.merchant_id(*input.merchant_filter);
        }
        if (input.constraints.get_mandate_status()) {
            db_query.status(input.constraints.get_mandate_status()->get_value());
        }
        if (const auto limit = input.constraints.get_limit(); limit && *limit > 0) {
            db_query.limit(static_cast<std::size_t>(*limit));
        }
        if (const auto offset = input.constraints.get_offset(); offset && *offset > 0) {
            db_query.offset(static_cast<std::size_t>(*offset));
        }
        const auto records = m_context.repository().find_mandates_by_customer_id(db_query);

        // ApplicationResponse::Json(mandates_list) -> bare array (core/mandate.rs:425).
        // from_db_mandate is applied to every row (core/mandate.rs:416-424); the
        // handler serializes the typed vector with Ser::serialize.
        std::vector<::app::model::mandates::MandateResponse> items{};
        items.reserve(records.size());
        const auto& connector_filter = input.constraints.get_connector();
        for (const auto& record: records) {
            if (connector_filter && !connector_filter->empty() &&
                record.get_connector() != *connector_filter) {
                continue;
            }
            if (!m_support.matches_created_time(record.get_created_at(), input.constraints)) {
                continue;
            }
            items.push_back(m_support.from_db_mandate(record));
        }
        return std::vector<::app::model::mandates::MandateResponse>{std::move(items)};
    }

private:
    ::app::context::AppContext& m_context;
    MandatesRuntimeSupport m_support;
};

} // namespace app::handlers::mandates
