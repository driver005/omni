export module app_payment:mandates_runtime_get_mandate;
import core_behavior;

import std;
import core_error;
import app_context;

import :mandates_input;
import :mandates_result;
import :mandates_runtime_common;

export namespace app::handlers::mandates {

class GetMandateRuntime
{
public:
    explicit GetMandateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: get_mandate ─────────────────────────
    // core/mandate.rs:33-58 (find + MandateResponse::from_db_mandate).
    [[nodiscard]] core::behavior::Outcome<::app::model::mandates::MandateResponse>
    get_mandate(const core::behavior::CallContext& ctx) const
    {
        const auto input = GetMandateData{ctx};
        m_support.record_flow(
            "MandatesRetrieve"
        ); // LockAction::NotApplicable (routes/mandates.rs:37)

        // platform.get_processor().get_account().get_id() is localised to the
        // authenticated merchant id (core/mandate.rs:43).
        const auto record =
            m_support.find_mandate_for_merchant(input.merchant_id, input.mandate_id);
        if (!record) {
            return std::unexpected(m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "Mandate does not exist in our records", // MandateNotFound
                "HE_02"
            ));
        }

        // MandateResponse::from_db_mandate (types/api/mandates.rs:39-135) is
        // encoded by the handler via serde::Ser::serialize into the exact wire shape.
        return ::app::model::mandates::MandateResponse{m_support.from_db_mandate(*record)};
    }

private:
    ::app::context::AppContext& m_context;
    MandatesRuntimeSupport m_support;
};

} // namespace app::handlers::mandates
