export module blog_handlers:posts_bulk_edit_step;
import :domain_all;
import :posts_support;

import std;
import core_workflow;
import :outbox_shim;
import core_serde;
import interfaces;

export namespace blog::handlers::posts::bulk_edit::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"posts_bulk_edit_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& input, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::posts::support;
        const auto parsed = parse(input.get_path(), kBulkOptions, true);
        if (!parsed.error.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", parsed.status}, {"body", parsed.error}}
            };
        }

        const auto raw = std::string{input.body_as_string()};
        const auto body = decode(raw);
        const Object* root = body && std::holds_alternative<Object>(body->get())
                                 ? &std::get<Object>(body->get())
                                 : nullptr;
        const std::string action = root == nullptr ? std::string{} : object_field(*root, "action");
        if (parsed.query.raw_filter.empty() || action.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"}, {"body", validation("action", "filter/action is required")}}
            };
        }

        auto& ctx = *s_ctx;
        const auto ids = ids_for_filter(ctx, parsed.query.raw_filter);
        const std::string patch = action_patch(action, root);
        if (patch.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"}, {"body", bad_request("Unsupported bulk action.")}}
            };
        }

        std::size_t successful = 0;
        std::size_t unsuccessful = 0;
        blog::handlers::outbox::global_outbox().begin_transaction();
        for (const auto& id: ids) {
            const auto result = ctx.store().edit("posts", id, patch);
            if (result.ok()) {
                ++successful;
            } else {
                ++unsuccessful;
            }
        }
        emit(
            "posts.bulk_edited",
            std::format(
                R"({{"action":{},"editIds":{},"successful":{},"unsuccessful":{}}})",
                quote(action),
                id_list(ids),
                successful,
                unsuccessful
            ),
            ""
        );
        blog::handlers::outbox::global_outbox().commit();

        const std::string response = std::format(
            R"({{"successful":{},"unsuccessful":{},"editIds":{}}})",
            successful,
            unsuccessful,
            id_list(ids)
        );
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", response}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    using Object = serde::Value::Object;

    [[nodiscard]] static std::string quote(std::string_view text)
    {
        return core_serde::JsonCodec{}.encode(std::string{text});
    }

    [[nodiscard]] static std::string action_patch(std::string_view action, const Object* root)
    {
        const Object* meta = nullptr;
        if (root != nullptr) {
            for (const auto& [name, value]: *root) {
                if (name == "meta" && std::holds_alternative<Object>(value.get())) {
                    meta = &std::get<Object>(value.get());
                }
            }
        }
        const auto meta_field = [&](std::string_view key)
        {
            return meta == nullptr ? std::string{}
                                   : blog::handlers::posts::support::object_field(*meta, key);
        };

        if (action == "unpublish") {
            return R"({"status":"draft"})";
        }
        if (action == "unschedule") {
            return R"({"status":"draft","published_at":null})";
        }
        if (action == "feature") {
            return R"({"featured":true})";
        }
        if (action == "unfeature") {
            return R"({"featured":false})";
        }
        if (action == "access") {
            const auto visibility = meta_field("visibility");
            return R"({"visibility":")" +
                   (visibility.empty() ? std::string{"public"} : visibility) + R"("})";
        }
        if (action == "addTag") {
            const auto tag = meta_field("tagIds");
            return R"({"tags":)" + (tag.empty() ? std::string{"[]"} : tag) + R"(})";
        }
        return {};
    }

    [[nodiscard]] static std::string id_list(const std::vector<std::string>& ids)
    {
        std::string out = "[";
        bool first = true;
        for (const auto& id: ids) {
            if (!first) {
                out += ',';
            }
            out += '"' + id + '"';
            first = false;
        }
        return out + ']';
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::posts::bulk_edit::workflow
