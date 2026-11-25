export module blog_handlers:snippets_create_step;
import :domain_all;
import :snippets_browse_step;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import interfaces;

export namespace blog::handlers::snippets::create::workflow {

namespace local_mapping {
    [[nodiscard]] inline std::string
    wrap(std::string_view resource, std::string_view rows, std::string_view meta = {})
    {
        return "{\"" + std::string{resource} + "\":" + std::string{rows} +
               (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
    }

    [[nodiscard]] inline std::string clean(std::string_view json, std::string_view)
    {
        return std::string{json};
    }

    [[nodiscard]] inline std::string pagination(
        std::size_t total,
        std::size_t limit,
        std::size_t page,
        std::size_t pages,
        std::string_view prev,
        std::string_view next
    )
    {
        return std::format(
            R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
            total,
            limit,
            page,
            pages,
            prev.empty() ? std::string{"null"} : std::string{prev},
            next.empty() ? std::string{"null"} : std::string{next}
        );
    }
} // namespace local_mapping

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"snippets_create_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::snippets::support;
        auto& ctx = *s_ctx;

        core::auth::ghost::GhostAuthorizer authorizer;
        const auto authorized = authorizer.authorize(principal_for(in), true, "snippets", "add");
        if (!authorized.allowed) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(authorized.status)}, {"body", authorized.error_json}}
            };
        }

        const auto q = core::query::parse(in.get_path(), kAddOptions);
        if (!formats_ok(q)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"}, {"body", formats_error()}}
            };
        }

        const std::string payload = unwrap_body(in.body_as_string());
        seed_snippet(ctx);

        // Ghost's unique-name constraint on snippets.name -> ValidationError.
        const std::string name = row_field(payload, "name");
        if (!name.empty()) {
            for (const auto& row:
                 blog::api::parse_array(ctx.store().resources().list("snippets"))) {
                if (row_field(blog::api::record_to_json(row), "name") == name) {
                    return core::workflow::StepResponse<Output>{
                        {{"status", "422"},
                         {"body",
                          blog::api::ApiError{}.validation(
                              "Snippet already exists.",
                              "Validation (Unique) failed for name",
                              "name"
                          )}}
                    };
                }
            }
        }

        const std::string id = ctx.store().resources().create("snippets", payload);
        const auto stored = ctx.store().resources().get("snippets", id);
        const auto cleaned = local_mapping::clean(with_formats("[" + stored + "]", q), "snippet");
        const auto body = local_mapping::wrap("snippets", cleaned);
        emit_action("added", id, body);
        return core::workflow::StepResponse<Output>{{{"status", "201"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::snippets::create::workflow
