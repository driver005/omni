export module blog_handlers:tags_destroy_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import :outbox_shim;
import interfaces;

export namespace blog::handlers::tags::destroy::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"tags_destroy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        constexpr std::array<std::string_view, 1> kOptions{"id"};
        (void)core::query::parse(in.get_path(), kOptions); // id required (options:['id'])

        const std::string path{in.get_path()};
        const auto base = path.substr(0, path.find('?'));
        const auto id = base.substr(base.rfind('/') + 1);
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}.validation("A tag id is required.", "Bad Request", "id")}}
            };
        }

        // Ghost Tag.destroy runs inside a transaction locking posts_tags; the shared
        // outbox supplies the context-local transaction + tag.deleted side effect.
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        const auto result = ctx.store().destroy("tags", id);
        if (!result.ok()) {
            outbox.rollback();
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(result.status())},
                 {"body", blog::api::ApiError{}.json("Tag not found.", "Not Found")}}
            };
        }
        outbox.emit(blog::handlers::outbox::Event{"tag.deleted", result.body(), id});
        outbox.commit();
        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(result.status())}, {"body", result.body()}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::tags::destroy::workflow
