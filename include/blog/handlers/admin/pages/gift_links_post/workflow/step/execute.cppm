export module blog_handlers:pages_gift_links_post_step;
import :domain_all;
import :gift_links_ops;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::pages::gift_links_post::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"pages_gift_links_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        const std::string id = blog::api::gift_links_ops::parse_resource_id(in.get_path());
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"}, {"body", blog::api::gift_links_ops::validation_error_body()}}
            };
        }
        const auto actor = blog::api::gift_links_ops::actor_from_request(in);
        const auto result = blog::api::gift_links_ops::reset(s_ctx->store(), id, "pages", actor);
        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(result.first)}, {"body", result.second}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::pages::gift_links_post::workflow
