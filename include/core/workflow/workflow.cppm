export module core_workflow:workflow;

import std;
import :engine;
import :value;
import :context;
import :definition;
import :execution;
import :registry;
import :runtime;
import :builder;
import :step_response;

export namespace core::workflow {

/**
 * @brief The only handle a consumer holds. Backend, driver and registry are
 * hidden behind a pImpl; definitions and execution go through this facade.
 * All members are inline — no free functions, no static member functions.
 */
class Workflow
{
public:
    Workflow() :
        Workflow(WorkflowEngine::IN_MEMORY)
    {
    }

    explicit Workflow(WorkflowEngine engine) :
        m_runtime{std::make_shared<WorkflowRuntime>(engine, nullptr)}
    {
    }

    void use_backend(std::unique_ptr<WorkflowDriver> driver)
    {
        m_runtime = std::make_shared<WorkflowRuntime>(m_runtime->engine(), std::move(driver));
    }

    Workflow(const Workflow&) = delete;
    Workflow& operator=(const Workflow&) = delete;
    Workflow(Workflow&&) noexcept = default;
    Workflow& operator=(Workflow&&) noexcept = default;
    ~Workflow() = default;

    [[nodiscard]] bool is_open() const noexcept
    {
        return m_runtime != nullptr;
    }

    [[nodiscard]] std::string_view engine_name() const noexcept
    {
        return m_runtime ? m_runtime->name() : "none";
    }

    void register_workflow(WorkflowDefinition definition)
    {
        m_runtime->registry().register_definition(std::move(definition));
    }

    [[nodiscard]] const WorkflowDefinition* find(std::string_view name) const noexcept
    {
        return m_runtime->registry().find(name);
    }

    [[nodiscard]] std::vector<std::string> workflow_names() const
    {
        return m_runtime->registry().names();
    }

    template<typename Input>
    [[nodiscard]] WorkflowExecution run(std::string_view name, Input&& input)
    {
        const WorkflowDefinition* definition = find(name);
        if (definition == nullptr) {
            throw std::invalid_argument{"core_workflow: unknown workflow " + std::string{name}};
        }
        WorkflowContext context;
        if constexpr (std::is_lvalue_reference_v<Input>) {
            context.set_input(std::any{std::ref(input)});
        } else {
            context.set_input(std::any{std::forward<Input>(input)});
        }
        return m_runtime->driver().run(*definition, context);
    }

    [[nodiscard]] WorkflowExecution run(std::string_view name)
    {
        return run(name, WorkflowValue{std::monostate{}});
    }

    bool set_step_success(WorkflowExecution& execution, std::string_view step_name, WorkflowValue output)
    {
        return m_runtime->driver().set_step_success(execution, step_name, std::move(output));
    }

    bool set_step_failure(WorkflowExecution& execution, std::string_view step_name, std::string message)
    {
        return m_runtime->driver().set_step_failure(execution, step_name, std::move(message));
    }

private:
    std::shared_ptr<WorkflowRuntime> m_runtime{};
};

} // namespace core::workflow

#ifdef CONGELADO_TEST
namespace core::workflow::tests {
using namespace boost::ut;

class DeleteStep
{
public:
    using Input = std::string;
    using Output = void;
    static constexpr std::string_view name{"delete"};

    StepResponse<Output> invoke(const Input& input, WorkflowContext& context)
    {
        deleted = input;
        return StepResponse<void>{input};
    }

    void compensate(const WorkflowValue& ids, WorkflowContext&)
    {
        restored = WorkflowValueAs{}.operator()<std::string>(ids);
    }

    static inline std::string deleted{};
    static inline std::string restored{};
};

suite<"core_workflow_facade"> core_workflow_facade_suite = []
{
    "runs a workflow and reverts on permanent failure"_test = []
    {
        const auto definition = WorkflowBuilder{}
                                    .name("delete-thing")
                                    .step<DeleteStep>()
                                    .build();

        Workflow workflow;
        workflow.register_workflow(definition);
        expect(workflow.is_open());
        expect(workflow.engine_name() == "in-memory");
        expect(workflow.workflow_names().size() == 1);

        const auto success = workflow.run("delete-thing", std::string{"id_1"});
        expect(success.succeeded());
        expect(DeleteStep::deleted == "id_1");
    };
};

} // namespace core::workflow::tests
#endif
