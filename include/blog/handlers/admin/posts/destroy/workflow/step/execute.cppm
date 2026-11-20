export module blog_handlers:posts_destroy_step;
import :domain_all;
import :posts_support;

import std;
import core_workflow;
import :outbox_shim;
import interfaces;

export namespace blog::handlers::posts::destroy::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"posts_destroy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::posts::support;
        const auto parsed = parse(input.get_path(), kDestroyOptions);
        if (!parsed.error.empty()) {
            return core::workflow::StepResponse<Output>{{{"status", parsed.status},
                {"body", parsed.error}, {"invalidate", "/*"}}};
        }
        if (const auto error = validate_options(parsed.query)) {
            return core::workflow::StepResponse<Output>{{{"status", "422"}, {"body", *error}}};
        }

        const auto id_it = parsed.query.options.find("id");
        const std::string id = id_it != parsed.query.options.end() && !id_it->second.empty()
                                   ? id_it->second : path_tail(input.get_path());
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{{{"status", "422"},
                {"body", validation("id", "id is required")}, {"invalidate", "/*"}}};
        }

        const auto existing = s_ctx->store().find_by_resource("posts", id);
        const bool draft = existing && field_of(*existing, "status") == "draft";

        blog::handlers::outbox::global_outbox().begin_transaction();
        const auto result = s_ctx->store().destroy("posts", id);
        if (!result.ok()) {
            blog::handlers::outbox::global_outbox().rollback();
            return core::workflow::StepResponse<Output>{{{"status", std::to_string(result.status())},
                {"body", blog::api::ApiError{}.json("Post not found.", "Not Found")},
                {"invalidate", "/*"}}};
        }
        emit("posts.deleted", R"({"id":")" + id + R"("})", id);
        blog::handlers::outbox::global_outbox().commit();

        return core::workflow::StepResponse<Output>{{{"status", "204"}, {"body", ""},
            {"invalidate", draft ? "" : "/*"}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace
