import std;
import core_workflow;

using namespace core::workflow;

namespace {

class Check
{
public:
    void equal(std::string_view actual, std::string_view expected, std::string_view label)
    {
        if (actual != expected) {
            std::println(std::cerr, "FAIL {}: expected '{}', got '{}'", label, expected, actual);
            ++m_failures;
        }
    }

    void yes(bool condition, std::string_view label)
    {
        if (!condition) {
            std::println(std::cerr, "FAIL {}", label);
            ++m_failures;
        }
    }

    int failures() const
    {
        return m_failures;
    }

private:
    int m_failures{0};
};

std::vector<std::string> g_log{};

class CreateStep
{
public:
    using Input = std::string;
    using Output = std::string;
    static constexpr std::string_view name{"create"};

    StepResponse<Output> invoke(const Input& input, WorkflowContext&)
    {
        g_log.push_back("invoke:create:" + input);
        return StepResponse<Output>{"id_" + input};
    }

    void compensate(const WorkflowValue& ids, WorkflowContext&)
    {
        g_log.push_back("compensate:create:" + WorkflowValueAs{}.operator()<std::string>(ids));
    }
};

class NotifyStep
{
public:
    using Input = std::string;
    using Output = void;
    static constexpr std::string_view name{"notify"};

    StepResponse<Output> invoke(const Input& input, WorkflowContext&)
    {
        g_log.push_back("invoke:notify:" + input);
        return StepResponse<void>{input};
    }

    void compensate(const WorkflowValue&, WorkflowContext&)
    {
        g_log.push_back("compensate:notify");
    }
};

class FailStep
{
public:
    using Input = std::string;
    using Output = void;
    static constexpr std::string_view name{"fail"};

    StepResponse<Output> invoke(const Input&, WorkflowContext&)
    {
        g_log.push_back("invoke:fail");
        return StepResponse<void>{}.permanent_failure("payment declined");
    }

    void compensate(const WorkflowValue&, WorkflowContext&)
    {
        g_log.push_back("compensate:fail");
    }
};

class SkipStep
{
public:
    using Input = std::string;
    using Output = void;
    static constexpr std::string_view name{"skip"};

    StepResponse<Output> invoke(const Input&, WorkflowContext&)
    {
        g_log.push_back("invoke:skip");
        return StepResponse<void>{}.skip();
    }

    void compensate(const WorkflowValue&, WorkflowContext&)
    {
        g_log.push_back("compensate:skip");
    }
};

void reset_log()
{
    g_log.clear();
}

std::string joined_log()
{
    return std::ranges::fold_left(g_log, std::string{}, [](std::string acc, const std::string& entry) {
        if (!acc.empty()) {
            acc += '|';
        }
        acc += entry;
        return acc;
    });
}

} // namespace

int main()
{
    static_assert(WorkflowStep<CreateStep>);
    static_assert(WorkflowStep<NotifyStep>);
    static_assert(WorkflowStep<FailStep>);
    static_assert(WorkflowStep<SkipStep>);

    Check check;

    // 1. Happy path with a typed step-output ref and a fired hook.
    {
        reset_log();
        auto definition = WorkflowBuilder{}
                              .name("create-thing")
                              .step<CreateStep>()
                              .step<NotifyStep>(StepOutputOf<std::string>{"create"})
                              .hook("thingCreated")
                              .build();
        bool hook_fired{false};
        definition.hooks[0].set_handler([&](const WorkflowValue&, WorkflowContext&) { hook_fired = true; });

        Workflow workflow;
        workflow.register_workflow(definition);
        const auto execution = workflow.run("create-thing", std::string{"acme"});
        check.yes(execution.succeeded(), "happy path succeeds");
        check.yes(execution.has_finished(), "happy path is terminal");
        check.yes(hook_fired, "hook fires on success");
        check.equal(joined_log(), "invoke:create:acme|invoke:notify:id_acme", "invoke order");
        check.yes(execution.result_of("create") != nullptr, "step result recorded");
    }

    // 2. Permanent failure triggers reverse compensation.
    {
        reset_log();
        const auto definition = WorkflowBuilder{}
                                    .name("failing-flow")
                                    .step<CreateStep>()
                                    .step<FailStep>(StepOutputOf<std::string>{"create"})
                                    .build();

        Workflow workflow;
        workflow.register_workflow(definition);
        const auto execution = workflow.run("failing-flow", std::string{"x"});
        check.yes(!execution.succeeded(), "failure does not succeed");
        check.yes(execution.state == WorkflowState::REVERTED, "failure reverts");
        check.yes(execution.errors.size() == 1, "one error recorded");
        check.equal(
            joined_log(),
            "invoke:create:x|invoke:fail|compensate:create:id_x",
            "compensation runs in reverse"
        );
    }

    // 3. Skip signal keeps the workflow successful.
    {
        reset_log();
        const auto definition = WorkflowBuilder{}
                                    .name("skip-flow")
                                    .step<SkipStep>()
                                    .build();
        Workflow workflow;
        workflow.register_workflow(definition);
        const auto execution = workflow.run("skip-flow", std::string{"ignored"});
        check.yes(execution.succeeded(), "skip keeps workflow successful");
        check.equal(joined_log(), "invoke:skip", "skipped step is not compensated");
    }

    // 4. Checkpoint merge takes the higher state_flow_order index, exactly like
    // Medusa: WAITING_TO_COMPENSATE (3) overwrites DONE (2).
    {
        WorkflowCheckpoint current;
        current.state = WorkflowState::DONE;
        WorkflowCheckpoint incoming;
        incoming.state = WorkflowState::WAITING_TO_COMPENSATE;
        current.merge(incoming);
        check.yes(
            current.state == WorkflowState::WAITING_TO_COMPENSATE,
            "merge advances to higher-precedence state"
        );
    }

    // 5. Idempotency key shape.
    {
        WorkflowContext ctx;
        ctx.set_workflow_name("delete-promotions");
        ctx.set_transaction_id("trx_1");
        ctx.set_step_name("delete-promotions");
        check.equal(
            ctx.idempotency_key("invoke"),
            "delete-promotions:trx_1:delete-promotions:invoke",
            "idempotency key"
        );
    }

    // 6. Duplicate step names are rejected at build time.
    {
        auto builder = WorkflowBuilder{}.name("dupe").step<CreateStep>().step<CreateStep>();
        bool threw{false};
        try {
            (void)builder.build();
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        check.yes(threw, "duplicate step names rejected");
    }

    return check.failures() == 0 ? 0 : 1;
}
