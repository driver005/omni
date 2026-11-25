export module core_workflow:driver;

import std;
import :value;
import :definition;
import :context;
import :execution;

// Internal interface partition. Re-exported by base.cppm for toolchain
// correctness only — namespace is not exported, so importers cannot name it.
export namespace core::workflow {

/** @brief Backend contract implemented by the in-process and distributed drivers. */
class WorkflowDriver
{
public:
    virtual ~WorkflowDriver() = default;
    WorkflowDriver() = default;
    WorkflowDriver(const WorkflowDriver&) = delete;
    WorkflowDriver& operator=(const WorkflowDriver&) = delete;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;

    virtual WorkflowExecution
    run(const WorkflowDefinition& definition, WorkflowContext& context) = 0;

    virtual bool set_step_success(
        WorkflowExecution& execution,
        std::string_view step_name,
        WorkflowValue output
    ) = 0;

    virtual bool set_step_failure(
        WorkflowExecution& execution,
        std::string_view step_name,
        std::string message
    ) = 0;
};

} // namespace core::workflow
