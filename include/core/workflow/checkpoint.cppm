export module core_workflow:checkpoint;

import std;
import :value;
import :state;
import :step_outcome;

export namespace core::workflow {

/** @brief Checkpointed state of one step, including both invoke and compensate sides. */
struct StepCheckpoint
{
    std::string name{};
    StepState invoke_state{StepState::NOT_STARTED};
    StepState compensate_state{StepState::DORMANT};
    StepStatus status{StepStatus::IDLE};
    int attempts{0};
    StepOutcome last_outcome{};
};

/**
 * @brief Serializable execution checkpoint. `merge` uses Medusa's precedence
 * ordering (DONE before WAITING_TO_COMPENSATE) so concurrent writers cannot
 * regress a workflow's state.
 */
class WorkflowCheckpoint
{
public:
    std::string workflow_name{};
    std::string transaction_id{};
    std::string run_id{};
    WorkflowState state{WorkflowState::NOT_STARTED};
    std::map<std::string, StepCheckpoint> steps{};
    std::map<std::string, WorkflowValue> context{};
    std::vector<std::string> errors{};

    void merge(const WorkflowCheckpoint& other)
    {
        const WorkflowStateOrder order;
        if (order(other.state) > order(state)) {
            state = other.state;
        }
        for (const auto& [name, incoming]: other.steps) {
            auto it = steps.find(name);
            if (it == steps.end()) {
                steps.emplace(name, incoming);
                continue;
            }
            StepCheckpoint& current = it->second;
            if (incoming.attempts > current.attempts) {
                current = incoming;
            }
        }
        for (const auto& [key, value]: other.context) {
            context.insert_or_assign(key, value);
        }
        for (const std::string& error: other.errors) {
            if (std::ranges::find(errors, error) == errors.end()) {
                errors.push_back(error);
            }
        }
    }

    [[nodiscard]] const StepCheckpoint* find_step(std::string_view name) const noexcept
    {
        const auto it = steps.find(std::string{name});
        return it == steps.end() ? nullptr : &it->second;
    }
};

} // namespace core::workflow

#ifdef CONGELADO_TEST
namespace core::workflow::tests {
using namespace boost::ut;

suite<"core_workflow_checkpoint"> core_workflow_checkpoint_suite = []
{
    "merge advances to the higher-precedence state and dedups errors"_test = []
    {
        WorkflowCheckpoint current;
        current.state = WorkflowState::DONE;

        WorkflowCheckpoint incoming;
        incoming.state = WorkflowState::WAITING_TO_COMPENSATE;
        incoming.errors.push_back("first");
        incoming.errors.push_back("second");

        current.merge(incoming);
        expect(current.state == WorkflowState::WAITING_TO_COMPENSATE);
        expect(current.errors.size() == 2);
    };

    "merge advances a lower state"_test = []
    {
        WorkflowCheckpoint current;
        current.state = WorkflowState::NOT_STARTED;
        WorkflowCheckpoint incoming;
        incoming.state = WorkflowState::INVOKING;
        current.merge(incoming);
        expect(current.state == WorkflowState::INVOKING);
    };
};

} // namespace core::workflow::tests
#endif
