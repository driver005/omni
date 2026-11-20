export module blog_handlers:labels_destroy_step;
import :domain_all;
import :labels_browse_step;
import std;
import core_workflow;
import core_query;
import :outbox_shim;
import interfaces;

export namespace blog::handlers::labels::destroy::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"labels_destroy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        auto& ctx = *s_ctx;

        constexpr std::array<std::string_view, 1> kOptions{"id"};
        (void)core::query::parse(in.get_path(), kOptions);  // options:['id'], require:true

        const auto id = last_segment(in.get_path());
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{{{"status", "422"},
                {"body", validation("Validation error, cannot destroy label.",
                                    "id is required", "id")}}};
        }
        const std::string numeric = numeric_id(id);
        const auto existing = ctx.store().resources().get("labels", numeric);
        if (existing.empty()) {
            return core::workflow::StepResponse<Output>{{{"status", "404"},
                {"body", not_found()}}};
        }

        // Ghost Label.destroy uses a transaction with a linked-member row lock
        // and detach. The shared outbox supplies the context-local transaction
        // and the label.deleted side effect; the in-memory store has no linked
        // rows to detach.
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        if (!ctx.store().resources().destroy("labels", numeric)) {
            outbox.rollback();
            return core::workflow::StepResponse<Output>{{{"status", "404"},
                {"body", not_found()}}};
        }
        outbox.emit(blog::handlers::outbox::Event{"label.deleted", existing, wire_id(numeric)});
        outbox.commit();

        return core::workflow::StepResponse<Output>{{{"status", "204"}, {"body", ""}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    [[nodiscard]] static std::string last_segment(std::string_view path)
    {
        const auto base = path.substr(0, path.find('?'));
        return std::string{base.substr(base.rfind('/') + 1)};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::labels::destroy::workflow
