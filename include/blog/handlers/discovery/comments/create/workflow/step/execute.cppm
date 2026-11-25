export module blog_handlers:comments_create_step;
import :domain_all;
import :comments_model_comment;
import :comments_model_member;
import :comments_model_wire_fixtures;
import :outbox_shim;
import std;
import core_workflow;
import core_serde;
import core_query;
import interfaces;

export namespace blog::handlers::comments::create::workflow {

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

namespace detail {

    inline constexpr std::string_view kIncludeOptions[]{"include"};
    inline constexpr std::string_view
        kAllowedInclude[]{"post", "member", "replies", "replies.member"};

    using Object = serde::Value::Object;

    [[nodiscard]] inline std::string unquote(std::string_view raw)
    {
        auto s = raw;
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) {
            s.remove_prefix(1);
        }
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
            s.remove_suffix(1);
        }
        if (s.size() >= 2 &&
            ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\''))) {
            s = s.substr(1, s.size() - 2);
        }
        return std::string{s};
    }

    [[nodiscard]] inline const serde::Value* value_for(const Object* obj, std::string_view key)
    {
        if (obj == nullptr) {
            return nullptr;
        }
        for (const auto& [name, value]: *obj) {
            if (name == key) {
                return &value;
            }
        }
        return nullptr;
    }

    [[nodiscard]] inline std::string scalar(const Object* obj, std::string_view key)
    {
        const auto* value = value_for(obj, key);
        if (value == nullptr) {
            return {};
        }
        if (const auto* text = std::get_if<std::string>(&value->get())) {
            return *text;
        }
        return {};
    }

    inline void set_text(Object& obj, std::string_view key, std::string value)
    {
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            if (it->first == key) {
                it->second = serde::Value{std::move(value)};
                return;
            }
        }
        obj[std::string{key}] = serde::Value{std::move(value)};
    }

    /** @brief Ghost validateCreatedAt: string/Date only, valid, not in the future. */
    [[nodiscard]] inline bool valid_created_at(const serde::Value* value)
    {
        if (value == nullptr) {
            return false;
        }
        const auto* text = std::get_if<std::string>(&value->get());
        if (text == nullptr || text->size() < 10) {
            return false;
        }
        if ((*text)[4] != '-' || (*text)[7] != '-') {
            return false;
        }
        for (std::size_t i = 0; i < 10; ++i) {
            if ((*text)[i] == '-') {
                continue;
            }
            if ((*text)[i] < '0' || (*text)[i] > '9') {
                return false;
            }
        }
        try {
            const int year = std::stoi(text->substr(0, 4));
            const auto now_year =
                std::chrono::year_month_day{
                    std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now())
                }
                    .year();
            return year <= static_cast<int>(now_year);
        } catch (...) {
            return false;
        }
    }

    [[nodiscard]] inline bool comments_disabled(const blog::BlogContext& ctx)
    {
        for (const auto& record: blog::api::parse_array(ctx.store().json_for("settings"))) {
            const auto key = record.find("key");
            const auto value = record.find("value");
            if (key == record.end() || value == record.end()) {
                continue;
            }
            if (unquote(key->second) == "comments_enabled" && unquote(value->second) == "off") {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] inline std::string
    invalidation(std::string_view post_id, std::string_view parent_id, std::string_view id)
    {
        std::vector<std::string> paths;
        if (!post_id.empty()) {
            paths.push_back("/api/members/comments/post/" + std::string{post_id} + "/");
        }
        if (!parent_id.empty()) {
            paths.push_back("/api/members/comments/" + std::string{parent_id} + "/replies/");
        }
        if (!id.empty()) {
            paths.push_back("/api/members/comments/" + std::string{id} + "/");
        }
        std::string out;
        for (const auto& path: paths) {
            if (!out.empty()) {
                out += ", ";
            }
            out += path;
        }
        return out;
    }

} // namespace detail

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"comments_create_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        const auto q = core::query::parse(in.get_path(), detail::kIncludeOptions);
        for (const auto& include: q.include) {
            if (std::ranges::find(detail::kAllowedInclude, include) ==
                std::ranges::end(detail::kAllowedInclude)) {
                return error("400", "Validation error", "Invalid include value", "include");
            }
        }

        std::string raw{in.body_as_string()};
        if (raw.empty()) {
            raw = "{}";
        }
        const auto decoded = core_serde::JsonCodec{}.decode<serde::Value>(raw);
        auto* root = decoded ? std::get_if<detail::Object>(&decoded->get()) : nullptr;
        detail::Object entity;
        bool unwrapped = false;
        if (root != nullptr) {
            const auto* data = detail::value_for(root, "data");
            const auto* data_obj =
                data == nullptr ? nullptr : std::get_if<detail::Object>(&data->get());
            if (data_obj != nullptr) {
                const auto* comments = detail::value_for(data_obj, "comments");
                const auto* array = comments == nullptr
                                        ? nullptr
                                        : std::get_if<serde::Value::Array>(&comments->get());
                if (array != nullptr && !array->empty()) {
                    if (const auto* first = std::get_if<detail::Object>(&array->front().get())) {
                        entity = *first;
                        unwrapped = true;
                    }
                }
            }
        }
        if (!unwrapped && root != nullptr) {
            entity = *root;
        }
        detail::Object* data = &entity;

        const auto html = detail::scalar(data, "html");
        if (html.empty()) {
            return error("400", "Validation error", "html is required", "html");
        }

        std::string member_id = detail::scalar(data, "member_id");
        if (member_id.empty() && !in.find_header("X-Member-Id").empty()) {
            member_id = std::string{in.find_header("X-Member-Id")};
        }
        if (member_id.empty()) {
            return error("400", "Validation error", "member_id is required", "member_id");
        }

        std::string post_id = detail::scalar(data, "post_id");
        const std::string parent_id = detail::scalar(data, "parent_id");
        if (post_id.empty() && parent_id.empty()) {
            return error(
                "400",
                "Validation error",
                "Either post_id (for top-level comments) or parent_id (for replies) must be "
                "provided",
                "post_id"
            );
        }

        if (detail::comments_disabled(ctx)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "405"},
                 {"body",
                  blog::api::ApiError{}.json("Comments are not enabled", "Method Not Allowed")}}
            };
        }

        std::string in_reply_to = detail::scalar(data, "in_reply_to_id");
        if (!parent_id.empty()) {
            const auto parents = blog::api::parse_array(ctx.store().json_for("comments"));
            const blog::api::RecordDto* parent = nullptr;
            for (const auto& record: parents) {
                const auto it = record.find("id");
                if (it != record.end() && detail::unquote(it->second) == parent_id) {
                    parent = &record;
                    break;
                }
            }
            if (parent == nullptr) {
                return core::workflow::StepResponse<Output>{
                    {{"status", "404"},
                     {"body", blog::api::ApiError{}.json("Resource not found", "Not Found")}}
                };
            }
            const auto grandparent = parent->find("parent_id");
            const auto self = parent->find("id");
            const std::string gp =
                grandparent == parent->end() ? std::string{} : detail::unquote(grandparent->second);
            const std::string self_id =
                self == parent->end() ? std::string{} : detail::unquote(self->second);
            if (!gp.empty() && gp != "null" && gp != self_id) {
                return error("400", "Validation error", "Cannot reply to a reply", "parent_id");
            }
            if (!in_reply_to.empty() && in_reply_to != parent_id) {
                const bool sibling = std::ranges::any_of(
                    parents,
                    [&](const auto& record)
                    {
                        const auto id = record.find("id");
                        const auto par = record.find("parent_id");
                        return id != record.end() && par != record.end() &&
                               detail::unquote(id->second) == in_reply_to &&
                               detail::unquote(par->second) == parent_id;
                    }
                );
                if (!sibling) {
                    in_reply_to.clear();
                }
            }
        }

        // Build the comment insert: Ghost forces status published and drops an
        // invalid created_at; context.internal suppresses notifications.
        detail::Object insert;
        for (const auto& [key, value]: *data) {
            if (key == "created_at") {
                if (detail::valid_created_at(&value)) {
                    insert[std::string{key}] = value;
                }
                continue;
            }
            if (key == "status") {
                continue;
            }
            insert[std::string{key}] = value;
        }
        detail::set_text(insert, "status", "published");
        detail::set_text(insert, "member_id", member_id);
        if (parent_id.empty()) {
            detail::set_text(insert, "post_id", post_id);
            detail::set_text(
                insert,
                "parent_id",
                ""
            ); // wire fixture maps empty -> own id; normalized to null on read
        } else {
            detail::set_text(insert, "parent_id", parent_id);
            detail::set_text(insert, "in_reply_to_id", in_reply_to);
        }
        const auto body = core_serde::JsonCodec{}.encode(serde::Value{insert});

        const auto result = ctx.store().create("comments", body);
        if (!result.ok()) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(result.status())}, {"body", result.body()}}
            };
        }

        const auto& created = result.body();
        std::string id;
        if (const auto pos = created.find("\"id\":\""); pos != std::string::npos) {
            const auto begin = pos + 6;
            const auto end = created.find('"', begin);
            if (end != std::string::npos) {
                id = created.substr(begin, end - begin);
            }
        }

        // DomainEvents.dispatch(MemberCommentEvent.create(...)) -> outbox.
        const auto quote = [](std::string_view text)
        {
            return "\"" + std::string{text} + "\"";
        };
        const auto payload = "{\"memberId\":" + quote(member_id) + ",\"postId\":" + quote(post_id) +
                             ",\"commentId\":" + quote(id) + "}";
        blog::handlers::outbox::global_outbox().emit(
            blog::handlers::outbox::Event{"MemberCommentEvent", payload, id}
        );

        // Ghost re-fetches the model so relations are present on the response.
        std::string wire;
        for (const auto& record: blog::api::parse_array(ctx.store().json_for("comments"))) {
            const auto it = record.find("id");
            if (it == record.end() || detail::unquote(it->second) != id) {
                continue;
            }
            wire = blog::api::record_to_json(record);
            break;
        }
        if (wire.empty()) {
            wire = "{}";
        }

        return core::workflow::StepResponse<Output>{
            {{"status", "201"},
             {"body", local_mapping::wrap("comments", local_mapping::clean(wire, "comment"))},
             {"invalidate", detail::invalidation(post_id, parent_id, id)}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static core::workflow::StepResponse<Output> error(
        std::string_view code,
        std::string_view message,
        std::string_view context,
        std::string_view property
    )
    {
        return core::workflow::StepResponse<Output>{
            {{"status", std::string{code}},
             {"body", blog::api::ApiError{}.validation(message, context, property)}}
        };
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::comments::create::workflow
