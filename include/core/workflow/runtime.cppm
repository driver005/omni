export module core_workflow:runtime;

import std;
import :engine;
import :registry;
import :driver;

export namespace core::workflow {
class WorkflowRuntime;
}

namespace core::workflow {

/** @brief Internal state owned by the Workflow facade (pImpl). */
class WorkflowRuntime
{
public:
    explicit WorkflowRuntime(WorkflowEngine engine, std::unique_ptr<WorkflowDriver> driver) :
        m_engine{engine},
        m_driver{std::move(driver)}
    {
    }

    WorkflowRuntime(const WorkflowRuntime&) = delete;
    WorkflowRuntime& operator=(const WorkflowRuntime&) = delete;

    [[nodiscard]] std::string_view name() const noexcept
    {
        return WorkflowEngineName{}(m_engine);
    }

    [[nodiscard]] WorkflowEngine engine() const noexcept
    {
        return m_engine;
    }

    [[nodiscard]] WorkflowDriver& driver() noexcept
    {
        return *m_driver;
    }

    [[nodiscard]] WorkflowRegistry& registry() noexcept
    {
        return m_registry;
    }

private:
    WorkflowEngine m_engine{WorkflowEngine::IN_MEMORY};
    std::unique_ptr<WorkflowDriver> m_driver{};
    WorkflowRegistry m_registry{};
};

} // namespace core::workflow
