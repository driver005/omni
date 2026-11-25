export module core_workflow:builder;

import std;
import :value;
import :context;
import :step;
import :step_outcome;
import :step_options;
import :definition;
import :hook;
import :expression;

export namespace core::workflow {


/** @brief Instance functor binding a typed step class into a type-erased descriptor. */
template<WorkflowStep S>
class StepBinder
{
public:
    template<typename Provider>
        requires std::invocable<Provider, WorkflowContext&>
    [[nodiscard]] StepDescriptor operator()(Provider provider, StepOptions options) const
    {
        StepDescriptor descriptor;
        descriptor.name = std::string{S::name};
        descriptor.options = options;

        descriptor.invoke = [provider =
                                 std::move(provider)](WorkflowContext& ctx) mutable -> StepOutcome
        {
            S step{};
            auto response = step.invoke(provider(ctx), ctx);
            StepOutcome outcome;
            outcome.signal = response.signal();
            outcome.output = response.to_value();
            outcome.typed_output = response.typed_output();
            outcome.compensate_input = response.compensate_input();
            outcome.message = response.message();
            return outcome;
        };

        descriptor.compensate = [](const WorkflowValue& input, WorkflowContext& ctx)
        {
            S step{};
            step.compensate(input, ctx);
        };

        return descriptor;
    }
};

/**
 * @brief The only public entry point for defining a workflow. Records steps once
 * at definition time, like Medusa's single-pass constructor run, and compiles
 * them into a WorkflowDefinition. No proxies — typed refs are explicit functors.
 */
class WorkflowBuilder
{
public:
    WorkflowBuilder& name(std::string name)
    {
        m_definition.name = std::move(name);
        return *this;
    }

    WorkflowBuilder& timeout(std::chrono::milliseconds limit)
    {
        m_definition.timeout = limit;
        return *this;
    }

    WorkflowBuilder& idempotent(bool enabled)
    {
        m_definition.idempotent = enabled;
        return *this;
    }

    WorkflowBuilder& retention(std::chrono::seconds duration)
    {
        m_definition.retention = duration;
        return *this;
    }

    /** @brief Adds a step whose input is the workflow input itself. */
    template<WorkflowStep S>
    WorkflowBuilder& step(StepOptions options = {})
    {
        return add<S>(InputOf<typename S::Input>{}, options);
    }

    /** @brief Adds a step with an explicit typed input provider. */
    template<WorkflowStep S, typename Provider>
        requires std::invocable<Provider, WorkflowContext&>
    WorkflowBuilder& step(Provider provider, StepOptions options = {})
    {
        return add<S>(std::move(provider), options);
    }

    WorkflowBuilder& hook(std::string name, WorkflowValue data = {})
    {
        HookDescriptor descriptor;
        descriptor.name = std::move(name);
        descriptor.data = std::move(data);
        m_definition.hooks.push_back(std::move(descriptor));
        return *this;
    }

    template<WorkflowHook H>
    WorkflowBuilder& hook(WorkflowValue data = {})
    {
        return hook(std::string{H::name}, std::move(data));
    }

    [[nodiscard]] WorkflowDefinition build() const
    {
        if (m_definition.name.empty()) {
            throw std::invalid_argument{"core_workflow: workflow name must not be empty"};
        }
        std::set<std::string> seen{};
        for (const StepDescriptor& step: m_definition.steps) {
            if (!seen.insert(step.name).second) {
                throw std::invalid_argument{"core_workflow: duplicate step name " + step.name};
            }
        }
        return m_definition;
    }

private:
    template<WorkflowStep S, typename Provider>
    WorkflowBuilder& add(Provider provider, StepOptions options)
    {
        m_definition.steps.push_back(StepBinder<S>{}(std::move(provider), options));
        return *this;
    }

    WorkflowDefinition m_definition{};
};

} // namespace core::workflow

#ifdef CONGELADO_TEST
namespace core::workflow::tests {
using namespace boost::ut;

class FirstStep
{
public:
    using Input = std::string;
    using Output = std::string;
    static constexpr std::string_view name{"first"};

    StepResponse<Output> invoke(const Input& input, WorkflowContext&)
    {
        return StepResponse<Output>{"id_" + input};
    }

    void compensate(const WorkflowValue&, WorkflowContext&) {}
};

class SecondStep
{
public:
    using Input = std::string;
    using Output = void;
    static constexpr std::string_view name{"second"};

    StepResponse<Output> invoke(const Input& input, WorkflowContext&)
    {
        expect(input == "id_acme");
        return StepResponse<void>{};
    }

    void compensate(const WorkflowValue&, WorkflowContext&) {}
};

suite<"core_workflow_builder"> core_workflow_builder_suite = []
{
    "compiles typed steps, refs and hooks into a definition"_test = []
    {
        const auto definition =
            WorkflowBuilder{}
                .name("create-thing")
                .step<FirstStep>()
                .step<SecondStep>(StepOutputOf<std::string>{"first"}, StepOptions{}.max_retries(2))
                .hook("thingCreated")
                .build();
        expect(definition.valid());
        expect(definition.steps.size() == 2);
        expect(definition.hooks.size() == 1);
        expect(definition.steps[1].options.max_retries() == 2);
    };

    "rejects duplicate step names"_test = []
    {
        auto builder = WorkflowBuilder{}.name("dupe").step<FirstStep>().step<FirstStep>();
        expect(
            throws<std::invalid_argument>(
                [&]
                {
                    (void)builder.build();
                }
            )
        );
    };
};

} // namespace core::workflow::tests
#endif
