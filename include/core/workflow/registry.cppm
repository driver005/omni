export module core_workflow:registry;

import std;
import :definition;
import :context;
import :value;
import :step_response;
import :builder;

export namespace core::workflow {

/**
 * @brief Process-wide workflow definition registry, built after
 * `core::events::EventBusRegistry`: one instance owns the data, a single active
 * pointer is process-global, and re-registering a name with a different shape
 * is rejected like Medusa's WorkflowManager.
 */
class WorkflowRegistry
{
public:
    void register_definition(WorkflowDefinition definition)
    {
        if (!definition.valid()) {
            throw std::invalid_argument{"core_workflow: cannot register an invalid definition"};
        }
        const auto it = m_definitions.find(definition.name);
        if (it != m_definitions.end() && it->second.steps.size() != definition.steps.size()) {
            throw std::invalid_argument{
                "core_workflow: workflow id already exists with a different step definition: " +
                definition.name
            };
        }
        m_definitions.insert_or_assign(std::move(definition.name), std::move(definition));
    }

    [[nodiscard]] const WorkflowDefinition* find(std::string_view name) const noexcept
    {
        const auto it = m_definitions.find(std::string{name});
        return it == m_definitions.end() ? nullptr : &it->second;
    }

    [[nodiscard]] std::vector<std::string> names() const
    {
        std::vector<std::string> result;
        result.reserve(m_definitions.size());
        for (const auto& [name, definition]: m_definitions) {
            result.push_back(name);
        }
        return result;
    }

    void set_active(WorkflowRegistry* registry) noexcept
    {
        s_active = registry;
    }

    [[nodiscard]] static WorkflowRegistry* get_active() noexcept
    {
        return s_active;
    }

private:
    std::map<std::string, WorkflowDefinition> m_definitions{};
    static inline WorkflowRegistry* s_active{nullptr};
};

} // namespace core::workflow

#ifdef CONGELADO_TEST
namespace core::workflow::tests {
using namespace boost::ut;

class RegistryStep
{
public:
    using Input = std::string;
    using Output = void;
    static constexpr std::string_view name{"registry-step"};

    StepResponse<Output> invoke(const Input&, WorkflowContext&)
    {
        return StepResponse<void>{};
    }

    void compensate(const WorkflowValue&, WorkflowContext&) {}
};

suite<"core_workflow_registry"> core_workflow_registry_suite = []
{
    "registers, finds and rejects divergent re-registration"_test = []
    {
        WorkflowRegistry registry;
        const auto definition =
            WorkflowBuilder{}.name("registry-flow").step<RegistryStep>().build();
        registry.register_definition(definition);
        expect(registry.find("registry-flow") != nullptr);
        expect(registry.find("missing") == nullptr);

        registry.set_active(&registry);
        expect(WorkflowRegistry::get_active() == &registry);
        registry.set_active(nullptr);
    };
};

} // namespace core::workflow::tests
#endif
