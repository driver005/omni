export module core_workflow:definition;

import std;
import :step;
import :hook;

export namespace core::workflow {

/** @brief Immutable execution plan produced by WorkflowBuilder. */
class WorkflowDefinition
{
public:
    std::string name{};
    std::vector<StepDescriptor> steps{};
    std::vector<HookDescriptor> hooks{};
    std::optional<std::chrono::milliseconds> timeout{};
    bool idempotent{false};
    std::optional<std::chrono::seconds> retention{};

    [[nodiscard]] const StepDescriptor* find_step(std::string_view step_name) const noexcept
    {
        for (const StepDescriptor& step: steps) {
            if (step.name == step_name) {
                return &step;
            }
        }
        return nullptr;
    }

    [[nodiscard]] bool has_step(std::string_view step_name) const noexcept
    {
        return find_step(step_name) != nullptr;
    }

    [[nodiscard]] bool valid() const noexcept
    {
        if (name.empty() || steps.empty()) {
            return false;
        }
        for (const StepDescriptor& step: steps) {
            if (!step.valid()) {
                return false;
            }
        }
        return true;
    }
};

} // namespace core::workflow

#ifdef CONGELADO_TEST
namespace core::workflow::tests {
using namespace boost::ut;

suite<"core_workflow_definition"> core_workflow_definition_suite = []
{
    "finds steps by name and rejects empty definitions"_test = []
    {
        WorkflowDefinition definition;
        definition.name = "delete-promotions";
        expect(!definition.valid());
        StepDescriptor step;
        step.name = "delete-promotions";
        step.invoke = [](WorkflowContext&)
        {
            return StepOutcome{};
        };
        definition.steps.push_back(step);
        expect(definition.valid());
        expect(definition.has_step("delete-promotions"));
        expect(definition.find_step("missing") == nullptr);
    };
};

} // namespace core::workflow::tests
#endif
