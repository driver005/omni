export module core_workflow:expression;

import std;
import :context;

export namespace core::workflow {

/** @brief Instance functor reading the typed workflow input. */
template<typename Input>
class InputOf
{
public:
    [[nodiscard]] const Input& operator()(const WorkflowContext& ctx) const
    {
        return ctx.template input_as<Input>();
    }
};

/** @brief Instance functor reading one typed field of a recorded step output. */
template<typename Output, typename Member>
class StepField
{
public:
    StepField(Member Output::* member, std::string step_name) :
        m_member{member},
        m_step_name{std::move(step_name)}
    {
    }

    [[nodiscard]] const Member& operator()(const WorkflowContext& ctx) const
    {
        return ctx.template step_output_as<Output>(m_step_name).*m_member;
    }

private:
    Member Output::* m_member{};
    std::string m_step_name{};
};

/** @brief Instance functor reading a whole typed step output. */
template<typename Output>
class StepOutputOf
{
public:
    explicit StepOutputOf(std::string step_name) :
        m_step_name{std::move(step_name)}
    {
    }

    [[nodiscard]] const Output& operator()(const WorkflowContext& ctx) const
    {
        return ctx.template step_output_as<Output>(m_step_name);
    }

private:
    std::string m_step_name{};
};

} // namespace core::workflow

#ifdef CONGELADO_TEST
namespace core::workflow::tests {
using namespace boost::ut;

struct Brand
{
    std::string id{};
};

suite<"core_workflow_expression"> core_workflow_expression_suite = []
{
    "reads input and step fields through instance functors"_test = []
    {
        WorkflowContext ctx;
        ctx.set_input(std::any{std::string{"Acme"}});
        ctx.set_step_output("create-brand", std::any{Brand{"brand_01"}});
        expect(InputOf<std::string>{}(ctx) == "Acme");
        expect(StepField<Brand, std::string>{&Brand::id, "create-brand"}(ctx) == "brand_01");
    };
};

} // namespace core::workflow::tests
#endif
