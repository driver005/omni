export module core_workflow:step;

import std;
import :value;
import :context;
import :state;
import :step_options;
import :step_response;
import :step_outcome;

export namespace core::workflow {

/**
 * @brief Compile-time contract for a Medusa-style step.
 *
 * A step is a plain class with public `Input`, `Output`, a unique compile-time
 * `name`, an `invoke` method and a `compensate` method. No macro and no ADL hook
 * is needed — the concept does all validation at compile time.
 */
template<typename S>
concept WorkflowStep = requires(S step, const typename S::Input& input, WorkflowContext& ctx) {
    typename S::Input;
    typename S::Output;
    { S::name } -> std::convertible_to<std::string_view>;
    { step.invoke(input, ctx) } -> std::convertible_to<StepResponse<typename S::Output>>;
    { step.compensate(std::declval<const WorkflowValue&>(), ctx) } -> std::same_as<void>;
};

/** @brief Type-erased step held by a workflow definition. */
class StepDescriptor
{
public:
    using InvokeFn = std::function<StepOutcome(WorkflowContext&)>;
    using CompensateFn = std::function<void(const WorkflowValue&, WorkflowContext&)>;

    std::string name{};
    StepOptions options{};
    InvokeFn invoke{};
    CompensateFn compensate{};

    [[nodiscard]] bool valid() const noexcept
    {
        return !name.empty() && invoke != nullptr;
    }
};

} // namespace core::workflow

#ifdef CONGELADO_TEST
namespace core::workflow::tests {
using namespace boost::ut;

class GoodStep
{
public:
    using Input = std::string;
    using Output = std::string;
    static constexpr std::string_view name{"good-step"};

    StepResponse<Output> invoke(const Input& input, WorkflowContext&)
    {
        return StepResponse<Output>{input};
    }

    void compensate(const WorkflowValue&, WorkflowContext&) {}
};

class MissingCompensateStep
{
public:
    using Input = std::string;
    using Output = std::string;
    static constexpr std::string_view name{"broken"};

    StepResponse<Output> invoke(const Input& input, WorkflowContext&)
    {
        return StepResponse<Output>{input};
    }
};

suite<"core_workflow_step"> core_workflow_step_suite = []
{
    "accepts a class with invoke and compensate"_test = []
    {
        static_assert(WorkflowStep<GoodStep>);
        expect(!WorkflowStep<MissingCompensateStep>);
    };

    "descriptor records name and callable"_test = []
    {
        StepDescriptor descriptor;
        descriptor.name = std::string{GoodStep::name};
        descriptor.invoke = [](WorkflowContext& ctx)
        {
            GoodStep step;
            const auto response = step.invoke(ctx.input_as<std::string>(), ctx);
            StepOutcome outcome;
            outcome.output = response.to_value();
            return outcome;
        };
        expect(descriptor.valid());
    };
};

} // namespace core::workflow::tests
#endif
