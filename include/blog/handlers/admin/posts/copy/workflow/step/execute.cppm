export module blog_handlers:posts_copy_step;
import :domain_all;
import :posts_support;

import std;
import core_workflow;
import core_serde;
import :outbox_shim;
import interfaces;

export namespace blog::handlers::posts::copy::workflow {

namespace local_mapping {
[[nodiscard]] inline std::string wrap(std::string_view resource, std::string_view rows,
                                      std::string_view meta = {})
{
    return "{\"" + std::string{resource} + "\":" + std::string{rows} +
           (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
}
[[nodiscard]] inline std::string clean(std::string_view json, std::string_view) { return std::string{json}; }
[[nodiscard]] inline std::string pagination(std::size_t total, std::size_t limit, std::size_t page,
                                            std::size_t pages, std::string_view prev,
                                            std::string_view next)
{
    return std::format(R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
                       total, limit, page, pages,
                       prev.empty() ? std::string{"null"} : std::string{prev},
                       next.empty() ? std::string{"null"} : std::string{next});
}
} // namespace local_mapping

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"posts_copy_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& input, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::posts::support;
        const auto parsed = parse(input.get_path(), kCopyOptions);
        if (!parsed.error.empty()) {
            return core::workflow::StepResponse<Output>{{{"status", parsed.status},
                {"body", parsed.error}}};
        }
        if (const auto error = validate_options(parsed.query)) {
            return core::workflow::StepResponse<Output>{{{"status", "422"}, {"body", *error}}};
        }

        const auto id_it = parsed.query.options.find("id");
        const std::string id = id_it != parsed.query.options.end() && !id_it->second.empty()
                                   ? id_it->second : path_tail(input.get_path());
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{{{"status", "422"},
                {"body", validation("id", "id is required")}}};
        }

        const auto source = s_ctx->store().find_by_resource("posts", id);
        if (!source) {
            return core::workflow::StepResponse<Output>{{{"status", "404"},
                {"body", blog::api::ApiError{}.json("Post not found.", "Not Found")}}};
        }

        const auto decoded = decode(*source);
        std::string json = R"json({"title":"(Copy)","status":"draft"})json";
        if (decoded && std::holds_alternative<Object>(decoded->get())) {
            const auto& object = std::get<Object>(decoded->get());
            std::string copied{"{"};
            for (const auto key : kCopiedFields) {
                const auto* value = value_for(object, key);
                if (value == nullptr) continue;
                if (copied.size() > 1) copied += ',';
                copied += core_serde::JsonCodec{}.encode(std::string{key}) + ":" +
                          core_serde::JsonCodec{}.encode(*value);
            }
            if (copied.size() > 1) copied += ',';
            copied += R"("title":)" + quote(object_field(object, "title") + " (Copy)") +
                      R"(,"status":"draft"})";
            json = copied;
        }

        auto& ctx = *s_ctx;
        blog::handlers::outbox::global_outbox().begin_transaction();
        const auto result = ctx.store().create("posts", json);
        const std::string new_id = first_post_id(result.body());
        emit("posts.copied",
             std::format(R"({{"source":{},"id":{}}})", quote(id), quote(new_id)), new_id);
        blog::handlers::outbox::global_outbox().commit();

        return core::workflow::StepResponse<Output>{{{"status", "201"},
            {"body", mapped(result.body())}, {"location", "/posts/" + new_id + "/"}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;

    // Ghost COPIED_POSTS_FIELDS + posts_meta fields (posts-service.js:24-55).
    inline static constexpr std::string_view kCopiedFields[]{
        "title", "mobiledoc", "lexical", "html", "plaintext", "feature_image", "featured",
        "show_title_and_feature_image", "type", "locale", "visibility",
        "email_recipient_filter", "custom_excerpt", "codeinjection_head",
        "codeinjection_foot", "custom_template", "meta_title", "meta_description",
        "og_image", "og_title", "og_description", "twitter_image", "twitter_title",
        "twitter_description", "feature_image_alt", "feature_image_caption",
        "tags", "authors", "tiers"
    };

    [[nodiscard]] static const serde::Value* value_for(const Object& object, std::string_view key)
    {
        for (const auto& [name, value] : object) {
            if (name == key) return &value;
        }
        return nullptr;
    }

    [[nodiscard]] static std::string quote(std::string_view text)
    {
        return core_serde::JsonCodec{}.encode(std::string{text});
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
                for (const auto& row : std::get<serde::Value::Array>(posts->get())) {
                    if (!first) rows += ',';
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

} // namespace
