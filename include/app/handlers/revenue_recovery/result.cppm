export module app_handlers_revenue_recovery:result;

import std;
import core_serde;
import app_model;

export namespace app::handlers::revenue_recovery {

// The cancel response inherits its wire fields from the exact model the old core
// built and encoded, so field names, order and wrapper types stay byte-identical.
// The Serializable specialization reuses that model's FieldDesc tuple; those
// descriptors apply to the derived object just as they do to the base object.
class RevenueRecoveryCancelResult : public app::model::payments::RecoveryPaymentsResponse
{
public:
    RevenueRecoveryCancelResult() = default;

    explicit RevenueRecoveryCancelResult(
        app::model::payments::RecoveryPaymentsResponse value
    ) noexcept :
        RecoveryPaymentsResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::revenue_recovery

template<>
struct serde::Serializable<app::handlers::revenue_recovery::RevenueRecoveryCancelResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::model::payments::RecoveryPaymentsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "revenue_recovery_cancel_result";
    }
};
