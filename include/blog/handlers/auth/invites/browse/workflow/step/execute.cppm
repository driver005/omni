export module blog_handlers:invites_browse_step;
import :domain_all;
import :invites_dto_admin_resources;
import std;
import core_workflow;
import core_query;
import interfaces;

export namespace blog::handlers::invites::browse::workflow {

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
    static constexpr std::string_view name{"invites_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        // Ghost `options` + `restrictAdminApiQueryOptions` (invites.js:22-30).
        // core::query parses the allowlist and guards restricted filter/order fields.
        static constexpr std::string_view
            kOptions[]{"include", "page", "limit", "fields", "filter", "order", "debug"};
        const auto q = core::query::parse(in.get_path(), kOptions);

        // validation.options.include = ALLOWED_INCLUDES = [] (invites.js:7,23-27).
        if (!q.include.empty()) {
            return invalid("include", "include is not supported");
        }

        const auto rows = ctx.store().json_for("invites", {});
        std::string ran;
        try {
            // clean applies Ghost's toJSON token stripping; run applies filter/order/
            // fields/page/limit and builds meta.pagination (crud.js findPage).
            ran = core::query::run(support::clean(rows), q, 15);
        } catch (const std::exception&) {
            // mongoTransformer / restricted `order` -> BadRequestError
            // (api-filter-utils.ts:275-289).
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body",
                  blog::api::ApiError{}
                      .json("Restricted fields cannot be used in order.", "Bad Request")}}
            };
        }

        const auto parts = blog::api::JsonBody{}.parse_raw(ran);
        const auto meta = blog::api::JsonBody{}.parse_raw(
            parts.contains("meta") ? parts.at("meta") : std::string{"{}"}
        );
        const auto page_meta = blog::api::JsonBody{}.parse_raw(
            meta.contains("pagination") ? meta.at("pagination") : std::string{"{}"}
        );
        const auto number = [&page_meta](const char* key, std::size_t fallback)
        {
            const auto it = page_meta.find(key);
            if (it == page_meta.end()) {
                return fallback;
            }
            try {
                return static_cast<std::size_t>(std::stoull(it->second));
            } catch (...) {
                return fallback;
            }
        };
        const std::string prev =
            page_meta.contains("prev") ? page_meta.at("prev") : std::string{"null"};
        const std::string next =
            page_meta.contains("next") ? page_meta.at("next") : std::string{"null"};
        const auto pagination = local_mapping::pagination(
            number("total", 0),
            number("limit", 15),
            number("page", 1),
            number("pages", 1),
            prev,
            next
        );
        const auto data = parts.contains("data") ? parts.at("data") : std::string{"[]"};
        const auto body = support::wrap(data, "{\"pagination\":" + pagination + "}");
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static core::workflow::StepResponse<Output>
    invalid(std::string_view property, std::string_view context)
    {
        return core::workflow::StepResponse<Output>{
            {{"status", "422"},
             {"body", blog::api::ApiError{}.validation("Validation error", context, property)}}
        };
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::invites::browse::workflow
