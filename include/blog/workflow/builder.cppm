export module blog_workflow:builder;

import std;
import core_workflow;
import memory_backends;
import :registry;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::workflow {

/** @brief Fluent entry point: builds a core workflow with all blog definitions. */
class BlogWorkflowBuilder
{
public:
    [[nodiscard]] core::workflow::Workflow build() const
    {
        core::workflow::Workflow workflow;
        workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
        BlogWorkflowRegistry{}.register_all(workflow);
        return workflow;
    }
};

} // namespace blog::workflow

#ifdef CONGELADO_TEST
namespace blog::workflow::tests {
using namespace boost::ut;
suite<"blog_workflow_builder"> blog_workflow_builder_suite = []
{
    "builds a workflow with all blog definitions"_test = []
    {
        auto workflow = BlogWorkflowBuilder{}.build();
        expect(workflow.workflow_names().size() == 4);
    };
};
} // namespace blog::workflow::tests
#endif
