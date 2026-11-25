export module blog_handlers:links_bulk_edit_step;
import :domain_all;
import :links_browse_step;
import std;
import core_workflow;
import core_query;
import core_serde;
import interfaces;

export namespace blog::handlers::links::bulk_edit::workflow {

namespace detail {

    /** @brief Read a string field from a decoded serde object. */
    [[nodiscard]] inline std::string field(const serde::Value& value, const char* key)
    {
        const auto object = value.to_object();
        if (!object) {
            return {};
        }
        const auto found = object->get(key);
        if (!found) {
            return {};
        }
        if (const auto* text = std::get_if<std::string>(&found->get())) {
            return *text;
        }
        return {};
    }

    /** @brief Read `meta.link.to` from the decoded `bulk`/`data` body scope. */
    [[nodiscard]] inline std::string link_to(const serde::Value& value)
    {
        const auto meta = value.to_object();
        if (!meta) {
            return {};
        }
        const auto meta_field = meta->get("meta");
        if (!meta_field) {
            return {};
        }
        const auto meta_object = meta_field->to_object();
        if (!meta_object) {
            return {};
        }
        const auto link = meta_object->get("link");
        if (!link) {
            return {};
        }
        return field(*link, "to");
    }

    /** @brief Ghost `frame.data.bulk` (legacy `data` scopes accepted as well). */
    [[nodiscard]] inline const serde::Value* bulk_scope(const serde::Value& root)
    {
        const auto object = root.to_object();
        if (!object) {
            return &root;
        }
        for (const char* key: {"bulk", "data"}) {
            const auto found = object->get(key);
            if (found && std::get_if<serde::Value::Object>(&found->get()) != nullptr) {
                return &*found;
            }
        }
        return &root;
    }

} // namespace detail

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"links_bulk_edit_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::links::support;
        auto& ctx = *s_ctx;
        auto& mutations = ctx.store();

        const auto q = core::query::parse(in.get_path(), kBrowseOptions);
        const std::string& filter = q.raw_filter;

        const std::string raw =
            in.body_as_string().empty() ? std::string{"{}"} : in.body_as_string();
        std::string action;
        std::string new_url;
        try {
            if (auto value = core_serde::JsonCodec{}.decode<serde::Value>(raw)) {
                const serde::Value* scope = detail::bulk_scope(*value);
                action = detail::field(*scope, "action");
                new_url = detail::link_to(*scope);
            }
        } catch (...) {
        }

        // Ghost controller validation: data.action required + values ['updateLink'],
        // options.filter required -> 422 ValidationError.
        if (filter.empty() || action != "updateLink") {
            const char* property =
                action.empty() ? "action" : (filter.empty() ? "filter" : "action");
            return core::workflow::StepResponse<Output>{
                {{"status", "422"}, {"body", validation_error(property)}}
            };
        }

        // Ghost #parseLinkFilter -> BadRequestError on an unparseable filter.
        const auto parsed = parse_link_filter(filter);
        if (!parsed) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"}, {"body", invalid_filter_error()}}
            };
        }
        if (new_url.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}
                      .validation("meta.link.to is required", "", "meta", "ValidationError")}}
            };
        }

        seed_link(ctx);

        // Ghost #updateLinks: transform the old URL, re-escape the NQL filter and
        // resolve distinct redirect ids through getFilteredIds.
        const std::string transformed_old = absolute_to_transform_ready(parsed->second);
        const std::string rebuilt = "post_id:" + escape_nql_string(parsed->first) +
                                    "+to:" + escape_nql_string(transformed_old);
        core::query::ParsedQuery ids_query;
        ids_query.raw_filter = rebuilt;
        std::string matched;
        try {
            matched = core::query::apply(link_rows(ctx), ids_query);
        } catch (const std::runtime_error&) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"}, {"body", invalid_filter_error()}}
            };
        }
        std::size_t matched_count = 0;
        const std::string ids = matched_ids(matched, matched_count);

        // Attribution + transform-ready target + moment() timestamp.
        const std::string attributed = with_attribution(new_url, parsed->second, parsed->first);
        const std::string final_to = absolute_to_transform_ready(attributed);

        blog::api::RecordDto update;
        update["ids"] = blog::api::quote_fragment(ids);
        update["to"] = blog::api::quote_fragment(final_to);
        update["updated_at"] = blog::api::quote_fragment(now_iso());

        const auto result = mutations.bulk_edit("links", blog::api::record_to_json(update));
        const std::size_t successful = updated_count(result.body());
        const std::size_t unsuccessful =
            matched_count > successful ? matched_count - successful : 0;

        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(result.status())},
             {"body", bulk_response(action, successful, unsuccessful)}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::links::bulk_edit::workflow
