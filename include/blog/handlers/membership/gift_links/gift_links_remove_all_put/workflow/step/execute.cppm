export module blog_handlers:gift_links_gift_links_remove_all_put_step;
import :domain_all;
import :gift_links_ops;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::gift_links::gift_links_remove_all_put::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"gift_links_gift_links_remove_all_put_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        const auto actor = blog::api::gift_links_ops::actor_from_request(in);
        const auto result = blog::api::gift_links_ops::remove_all(s_ctx->store(), actor);
        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(result.first)}, {"body", result.second}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::gift_links::gift_links_remove_all_put::workflow
