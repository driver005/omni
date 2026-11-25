export module app_handlers_three_ds_decision_rule:result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::three_ds_decision_rule {

// The wire fields come from the exact DTO the old core built and encoded, so the
// field name, order and wrapper type stay byte-identical. The Serializable
// specialization reuses the DTO's FieldDesc tuple; those descriptors apply to
// the derived object just as they do to the base object.
class ExecuteDecisionRuleResult :
    public app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteResponse
{
public:
    ExecuteDecisionRuleResult() = default;

    explicit ExecuteDecisionRuleResult(
        app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteResponse value
    ) noexcept :
        ThreeDsDecisionRuleExecuteResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::three_ds_decision_rule

template<>
struct serde::Serializable<app::handlers::three_ds_decision_rule::ExecuteDecisionRuleResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            app::dto::three_ds_decision_rule::ThreeDsDecisionRuleExecuteResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "execute_decision_rule_result";
    }
};
