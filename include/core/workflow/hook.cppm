export module core_workflow:hook;

import std;
import :value;
import :context;

export namespace core::workflow {

/** @brief A workflow hook: no-op when unconsumed, exactly one handler allowed. */
class HookDescriptor
{
public:
    using Handler = std::function<void(const WorkflowValue&, WorkflowContext&)>;

    std::string name{};
    WorkflowValue data{};
    Handler handler{};

    [[nodiscard]] bool has_handler() const noexcept
    {
        return handler != nullptr;
    }

    /** @return false when a handler was already attached (Medusa throws in that case). */
    bool set_handler(Handler handler)
    {
        if (this->handler) {
            return false;
        }
        this->handler = std::move(handler);
        return true;
    }
};

/** @brief Compile-time contract for a typed hook definition. */
template<typename H>
concept WorkflowHook = requires(H hook, const WorkflowValue& data, WorkflowContext& ctx) {
    { H::name } -> std::convertible_to<std::string_view>;
    { hook.invoke(data, ctx) } -> std::same_as<void>;
};

} // namespace core::workflow

#ifdef CONGELADO_TEST
namespace core::workflow::tests {
using namespace boost::ut;

suite<"core_workflow_hook"> core_workflow_hook_suite = []
{
    "accepts one handler and rejects a second"_test = []
    {
        HookDescriptor hook;
        hook.name = "promotionsDeleted";
        expect(!hook.has_handler());
        expect(hook.set_handler([](const WorkflowValue&, WorkflowContext&) {}));
        expect(hook.has_handler());
        expect(!hook.set_handler([](const WorkflowValue&, WorkflowContext&) {}));
    };
};

} // namespace core::workflow::tests
#endif
