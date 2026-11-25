export module core_workflow:state;

import std;

export namespace core::workflow {

/** @brief Workflow-level state. Ordering matches Medusa's state_flow_order. */
enum class WorkflowState
{
    NOT_STARTED,
    INVOKING,
    DONE,
    WAITING_TO_COMPENSATE,
    COMPENSATING,
    REVERTED,
    FAILED
};

/** @brief Per-step state, including the dormant compensation side. */
enum class StepState
{
    NOT_STARTED,
    INVOKING,
    COMPENSATING,
    DONE,
    REVERTED,
    FAILED,
    DORMANT,
    SKIPPED,
    SKIPPED_FAILURE,
    TIMEOUT
};

/** @brief Per-step status, orthogonal to state. */
enum class StepStatus
{
    IDLE,
    OK,
    WAITING,
    TEMPORARY_FAILURE,
    PERMANENT_FAILURE
};

/** @brief Control-flow signals a step can return instead of a plain result. */
enum class StepSignal
{
    NONE,
    PERMANENT_FAILURE,
    SKIP
};

/** @brief Instance functor: total ordering used when merging checkpoints. */
class WorkflowStateOrder
{
public:
    [[nodiscard]] int operator()(WorkflowState state) const noexcept
    {
        switch (state) {
            case WorkflowState::NOT_STARTED:
                return 0;
            case WorkflowState::INVOKING:
                return 1;
            case WorkflowState::DONE:
                return 2;
            case WorkflowState::WAITING_TO_COMPENSATE:
                return 3;
            case WorkflowState::COMPENSATING:
                return 4;
            case WorkflowState::REVERTED:
                return 5;
            case WorkflowState::FAILED:
                return 6;
        }
        return -1;
    }
};

/** @brief Instance functor: checks terminal workflow states. */
class IsWorkflowTerminal
{
public:
    [[nodiscard]] bool operator()(WorkflowState state) const noexcept
    {
        return state == WorkflowState::DONE || state == WorkflowState::REVERTED ||
               state == WorkflowState::FAILED;
    }
};

/** @brief Instance functor: checks forward-movement step states. */
class CanMoveForward
{
public:
    [[nodiscard]] bool operator()(StepState state) const noexcept
    {
        return state == StepState::DONE || state == StepState::FAILED ||
               state == StepState::TIMEOUT || state == StepState::SKIPPED ||
               state == StepState::SKIPPED_FAILURE;
    }
};

/** @brief Instance functor: checks backward (compensation) movement states. */
class CanMoveBackward
{
public:
    [[nodiscard]] bool operator()(StepState state) const noexcept
    {
        return state == StepState::DONE || state == StepState::REVERTED ||
               state == StepState::FAILED || state == StepState::DORMANT ||
               state == StepState::SKIPPED;
    }
};

} // namespace core::workflow

#ifdef CONGELADO_TEST
namespace core::workflow::tests {
using namespace boost::ut;

suite<"core_workflow_state"> core_workflow_state_suite = []
{
    "DONE sorts before WAITING_TO_COMPENSATE"_test = []
    {
        expect(
            WorkflowStateOrder{}(WorkflowState::DONE) <
            WorkflowStateOrder{}(WorkflowState::WAITING_TO_COMPENSATE)
        );
        expect(IsWorkflowTerminal{}(WorkflowState::REVERTED));
        expect(!IsWorkflowTerminal{}(WorkflowState::INVOKING));
    };

    "forward and backward guards differ on TIMEOUT/DORMANT"_test = []
    {
        expect(CanMoveForward{}(StepState::TIMEOUT));
        expect(!CanMoveBackward{}(StepState::TIMEOUT));
        expect(CanMoveBackward{}(StepState::DORMANT));
        expect(!CanMoveForward{}(StepState::DORMANT));
    };
};

} // namespace core::workflow::tests
#endif
