export module app_handlers_mandates:runtime_get_mandate;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::mandates {

class GetMandateRuntime
{
public:
    explicit GetMandateRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: get_mandate ─────────────────────────
    // core/mandate.rs:33-58 (find + MandateResponse::from_db_mandate).
    [[nodiscard]] std::expected<GetMandateResult, core::error::AppError>
    get_mandate(const GetMandateInput& input) const
    {
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
        return GetMandateResult{m_support.from_db_mandate(*record)};
    }

private:
    app::context::AppContext& m_context;
    MandatesRuntimeSupport m_support;
};

} // namespace app::handlers::mandates
