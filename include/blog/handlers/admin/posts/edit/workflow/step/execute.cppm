export module blog_handlers:posts_edit_step;
import :domain_all;
import :posts_support;

import std;
import core_workflow;
import core_serde;
import :outbox_shim;
import interfaces;

export namespace blog::handlers::posts::edit::workflow {

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
    static constexpr std::string_view name{"posts_edit_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& input, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::posts::support;
        const auto parsed = parse(input.get_path(), kEditOptions);
        if (!parsed.error.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", parsed.status}, {"body", parsed.error}}
            };
        }
        if (const auto error = validate_options(parsed.query)) {
            return core::workflow::StepResponse<Output>{{{"status", "422"}, {"body", *error}}};
        }

        const auto id_it = parsed.query.options.find("id");
        const std::string id = id_it != parsed.query.options.end() && !id_it->second.empty()
                                   ? id_it->second
                                   : path_tail(input.get_path());
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"}, {"body", validation("id", "id is required")}}
            };
        }

        auto& ctx = *s_ctx;
        const auto previous = ctx.store().find_by_resource("posts", id);
        if (!previous) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"},
                 {"body", blog::api::ApiError{}.json("Post not found.", "Not Found")}}
            };
        }
        const std::string old_status = field_of(*previous, "status");
        const std::string uuid = field_of(*previous, "uuid");

        const auto raw = std::string{input.body_as_string()};
        std::string post_json = raw.empty() ? std::string{"{}"} : raw;
        const auto decoded = decode(raw);
        if (decoded && std::holds_alternative<Object>(decoded->get())) {
            const auto& root = std::get<Object>(decoded->get());
            if (const auto data = value_for(root, "data")) {
                if (std::holds_alternative<Object>(data->get())) {
                    if (const auto posts = value_for(std::get<Object>(data->get()), "posts")) {
                        if (std::holds_alternative<serde::Value::Array>(posts->get())) {
                            const auto& array = std::get<serde::Value::Array>(posts->get());
                            if (!array.empty() &&
                                std::holds_alternative<Object>(array.front().get())) {
                                post_json = core_serde::JsonCodec{}.encode(array.front());
                            }
                        }
                    }
                }
            }
            if (const auto error = unsafe_attrs_error(const_cast<Input&>(input), post_json)) {
                return core::workflow::StepResponse<Output>{{{"status", "403"}, {"body", *error}}};
            }
        }
        const std::string new_status = field_of(post_json, "status");
        const std::string event = change_event(old_status, new_status, uuid);

        blog::handlers::outbox::global_outbox().begin_transaction();
        const auto result = ctx.store().edit("posts", id, post_json);
        emit(
            "posts.edited",
            std::format(
                R"({{"id":{},"event":{},"status":{}}})",
                quote(id),
                quote(event),
                quote(new_status.empty() ? old_status : new_status)
            ),
            id
        );
        blog::handlers::outbox::global_outbox().commit();

        return core::workflow::StepResponse<Output>{
            {{"status", "200"},
             {"body", mapped(result.body())},
             {"invalidate", cache_header(event, uuid)}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;

    [[nodiscard]] static const serde::Value* value_for(const Object& object, std::string_view key)
    {
        for (const auto& [name, value]: object) {
            if (name == key) {
                return &value;
            }
        }
        return nullptr;
    }

    [[nodiscard]] static std::string quote(std::string_view text)
    {
        return core_serde::JsonCodec{}.encode(std::string{text});
    }

    /** @brief Ghost `getChanges()` event string — only emitted when status changed. */
    [[nodiscard]] static std::string
    change_event(std::string_view old_status, std::string_view new_status, std::string_view uuid)
    {
        if (new_status.empty() || new_status == old_status) {
            return {};
        }
        if (new_status == "published") {
            return "published_updated";
        }
        if (old_status == "published") {
            return "unpublished";
        }
        if (new_status == "scheduled") {
            return "scheduled_updated";
        }
        if (new_status == "draft") {
            return "draft_updated";
        }
        (void)uuid;
        return {};
    }

    [[nodiscard]] static std::string cache_header(std::string_view event, std::string_view uuid)
    {
        if (event == "published_updated" || event == "unpublished") {
            return "/*";
        }
        if (event == "scheduled_updated" || event == "draft_updated") {
            if (uuid.empty()) {
                return {};
            }
            const std::string base = "/p/" + std::string{uuid} + "/";
            return base + ", " + base + "?member_status=anonymous, " + base +
                   "?member_status=free, " + base + "?member_status=paid";
        }
        return {};
    }

    [[nodiscard]] static std::string mapped(std::string_view body)
    {
        using namespace blog::handlers::posts::support;
        const auto parsed = decode(body);
        if (parsed && std::holds_alternative<Object>(parsed->get())) {
            const auto* posts = value_for(std::get<Object>(parsed->get()), "posts");
            if (posts != nullptr && std::holds_alternative<serde::Value::Array>(posts->get())) {
                std::string rows = "[";
                bool first = true;
                for (const auto& row: std::get<serde::Value::Array>(posts->get())) {
                    if (!first) {
                        rows += ',';
                    }
                    rows += local_mapping::clean(core_serde::JsonCodec{}.encode(row), "post");
                    first = false;
                }
                rows += ']';
                return local_mapping::wrap("posts", rows);
            }
        }
        return std::string{body};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::posts::edit::workflow
