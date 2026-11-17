export module core_workflow:engine;

import std;

export namespace core::workflow {

/** @brief Selects the workflow backend, mirroring CronEngine / StorageEngine. */
enum class WorkflowEngine { IN_MEMORY, DISTRIBUTED };

/** @brief Instance functor naming an engine. */
class WorkflowEngineName
{
public:
    [[nodiscard]] std::string_view operator()(WorkflowEngine engine) const noexcept
    {
        switch (engine) {
            case WorkflowEngine::IN_MEMORY: return "in-memory";
            case WorkflowEngine::DISTRIBUTED: return "distributed";
        }
        return "unknown";
    }
};

} // namespace core::workflow

#ifdef CONGELADO_TEST
namespace core::workflow::tests {
using namespace boost::ut;

suite<"core_workflow_engine"> core_workflow_engine_suite = []
{
    "names both engines"_test = []
    {
        expect(WorkflowEngineName{}(WorkflowEngine::IN_MEMORY) == "in-memory");
        expect(WorkflowEngineName{}(WorkflowEngine::DISTRIBUTED) == "distributed");
    };
};

} // namespace core::workflow::tests
#endif
