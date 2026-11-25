export module core_workflow:context;

import std;
import :value;

export namespace core::workflow {

/**
 * @brief Per-execution context: typed workflow input, typed step outputs and a
 * serializable data bag. Mirrors Medusa's StepExecutionContext plus the
 * transaction context used by `resolveValue`.
 *
 * Typed values live in std::any so steps keep their concrete Input/Output types
 * without forcing a runtime reflection scheme. The data bag is the serializable
 * part that gets checkpointed.
 */
class WorkflowContext
{
public:
    void set_input(std::any input)
    {
        m_input = std::move(input);
    }

    template<typename T>
    [[nodiscard]] const T& input_as() const
    {
        return std::any_cast<const T&>(m_input);
    }

    [[nodiscard]] bool has_input() const noexcept
    {
        return m_input.has_value();
    }

    void set_step_output(std::string name, std::any output)
    {
        m_step_outputs.insert_or_assign(std::move(name), std::move(output));
    }

    template<typename T>
    [[nodiscard]] const T& step_output_as(std::string_view name) const
    {
        const auto it = m_step_outputs.find(std::string{name});
        if (it == m_step_outputs.end()) {
            throw std::out_of_range{
                "core_workflow: no output recorded for step " + std::string{name}
            };
        }
        return std::any_cast<const T&>(it->second);
    }

    [[nodiscard]] bool has_step_output(std::string_view name) const noexcept
    {
        return m_step_outputs.contains(std::string{name});
    }

    void set(std::string key, WorkflowValue value)
    {
        m_data.insert_or_assign(std::move(key), std::move(value));
    }

    [[nodiscard]] WorkflowValue get(std::string_view key) const
    {
        const auto it = m_data.find(std::string{key});
        return it == m_data.end() ? WorkflowValue{std::monostate{}} : it->second;
    }

    void set_workflow_name(std::string name)
    {
        m_workflow_name = std::move(name);
    }

    [[nodiscard]] std::string_view workflow_name() const noexcept
    {
        return m_workflow_name;
    }

    void set_transaction_id(std::string id)
    {
        m_transaction_id = std::move(id);
    }

    [[nodiscard]] std::string_view transaction_id() const noexcept
    {
        return m_transaction_id;
    }

    void set_run_id(std::string id)
    {
        m_run_id = std::move(id);
    }

    [[nodiscard]] std::string_view run_id() const noexcept
    {
        return m_run_id;
    }

    void set_step_name(std::string name)
    {
        m_step_name = std::move(name);
    }

    [[nodiscard]] std::string_view step_name() const noexcept
    {
        return m_step_name;
    }

    void set_attempt(int attempt) noexcept
    {
        m_attempt = attempt;
    }

    [[nodiscard]] int attempt() const noexcept
    {
        return m_attempt;
    }

    /** @brief Medusa-style idempotency key: workflow:transaction:step:action. */
    [[nodiscard]] std::string idempotency_key(std::string_view action) const
    {
        return std::format("{}:{}:{}:{}", m_workflow_name, m_transaction_id, m_step_name, action);
    }

    [[nodiscard]] const std::map<std::string, WorkflowValue>& data() const noexcept
    {
        return m_data;
    }

private:
    std::any m_input{};
    std::map<std::string, std::any> m_step_outputs{};
    std::map<std::string, WorkflowValue> m_data{};
    std::string m_workflow_name{};
    std::string m_transaction_id{};
    std::string m_run_id{};
    std::string m_step_name{};
    int m_attempt{0};
};

} // namespace core::workflow

#ifdef CONGELADO_TEST
namespace core::workflow::tests {
using namespace boost::ut;

suite<"core_workflow_context"> core_workflow_context_suite = []
{
    "carries typed input and step outputs"_test = []
    {
        WorkflowContext ctx;
        ctx.set_input(std::any{std::string{"prom_01"}});
        expect(ctx.input_as<std::string>() == "prom_01");
        ctx.set_step_output("create-brand", std::any{std::int64_t{42}});
        expect(ctx.step_output_as<std::int64_t>("create-brand") == 42);
    };

    "builds Medusa-style idempotency key"_test = []
    {
        WorkflowContext ctx;
        ctx.set_workflow_name("delete-promotions");
        ctx.set_transaction_id("trx_1");
        ctx.set_step_name("delete-promotions");
        expect(ctx.idempotency_key("invoke") == "delete-promotions:trx_1:delete-promotions:invoke");
    };
};

} // namespace core::workflow::tests
#endif
