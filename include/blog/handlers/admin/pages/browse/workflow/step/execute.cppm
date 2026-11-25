export module blog_handlers:pages_browse_step;
import :domain_all;
import :pages_dto_links;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import interfaces;

export namespace blog::handlers::pages::browse::workflow {

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
    static constexpr std::string_view name{"pages_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::pages::support;
        auto& ctx = *s_ctx;

        // Ghost: mw.authAdminApi / mw.authenticatePublic + permissions.
        const auto principal = principal_for(in);
        core::auth::ghost::GhostAuthorizer authorizer;
        const auto authorized = authorizer.authorize(principal, true, "pages", "browse");
        if (!authorized.allowed) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(authorized.status)}, {"body", authorized.error_json}}
            };
        }

        // Ghost input pipeline: allow-listed options, restricted-field guard, NQL,
        // order, fields projection, page/limit/offset pagination.
        const auto q = core::query::parse(in.get_path(), kBrowseOptions);
        const bool pub = is_public(principal);
        const auto& allowedIncludes = pub ? std::span<const std::string_view>{kPublicIncludes}
                                          : std::span<const std::string_view>{kAdminIncludes};
        if (!includes_ok(q.include, allowedIncludes) ||
            !tokens_ok(q.options.contains("formats") ? q.options.at("formats") : "", kFormats)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body",
                  blog::api::ApiError{}
                      .validation("Invalid include or formats", "Validation Error", "include")}}
            };
        }

        // The Content API only serves published pages; the Admin API forces type:page
        // (every row in this store is already a page, so no type predicate is needed).
        auto scoped = q;
        if (pub) {
            scoped.filters.emplace_back("status", "published");
        }

        std::string run;
        try {
            run = core::query::run(ctx.store().json_for("pages", {}), scoped, 15);
        } catch (const std::exception&) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body",
                  blog::api::ApiError{}.validation(
                      "Restricted field used in Ghost query",
                      "Validation Error",
                      "filter"
                  )}}
            };
        }

        std::string data = "[]", meta = "{}";
        try {
            if (auto value = core_serde::JsonCodec{}.decode<serde::Value>(run)) {
                if (auto object = value->to_object()) {
                    if (const auto d = object->get("data")) {
                        data = core_serde::JsonCodec{}.encode(*d);
                    }
                    if (const auto m = object->get("meta")) {
                        meta = core_serde::JsonCodec{}.encode(*m);
                    }
                }
            }
        } catch (...) {
        }

        // Ghost output serializer: mappers.pages + clean + {pages, meta.pagination}.
        const auto cleaned = local_mapping::clean(data, "page");
        const auto body = local_mapping::wrap("pages", cleaned, meta);
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::pages::browse::workflow
