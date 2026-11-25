export module blog_handlers:search_index_search_index_authors_get_step;
import :domain_all;
import :search_index_dto_more_resources;
import std;
import core_workflow;
import interfaces;
import core_query;
import core_auth;

export namespace blog::handlers::search_index::search_index_authors_get::workflow {

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
    static constexpr std::string_view name{"search_index_search_index_authors_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::dto::search_index;
        auto& ctx = *s_ctx;

        // Content-only route: mw.authenticatePublic + permissions:true. Ghost
        // authorizes an anonymous Content API read of public content.
        const auto principal = principal_for(in);
        const auto spec = ghost_options("authors", true);
        core::auth::ghost::GhostAuthorizer authorizer;
        const auto authorized = authorizer.authorize(principal, spec.doc_name, "browse");
        if (!authorized.allowed) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(authorized.status)}, {"body", authorized.error_json}}
            };
        }

        const auto q = query_for(spec);
        std::string rows;
        try {
            // Author.findPage (search-index-public.js:52) -> the "authors" store,
            // not the "users" store used by the admin fetchUsers operation.
            rows = core::query::apply(ctx.store().json_for(spec.resource, {}), q, 10'000);
        } catch (const std::exception&) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body",
                  R"({"errors":[{"message":"Validation Error","type":"ValidationError"}]})"}}
            };
        }

        // Ghost output/search-index.js fetchAuthors -> {"authors":[...]}, no meta.
        const auto body = local_mapping::wrap("authors", local_mapping::clean(rows, spec.entity));
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::search_index::search_index_authors_get::workflow
