export module blog_handlers:comments_edit_step;
import :domain_all;
import :comments_model_comment;
import :comments_model_member;
import :comments_model_wire_fixtures;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::comments::edit::workflow {

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

namespace detail {

using Object = serde::Value::Object;

[[nodiscard]] inline std::string unquote(std::string_view raw)
{
    auto s = raw;
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.remove_prefix(1);
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.remove_suffix(1);
    if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') ||
                          (s.front() == '\'' && s.back() == '\''))) {
        s = s.substr(1, s.size() - 2);
    }
    return std::string{s};
}

[[nodiscard]] inline const serde::Value* value_for(const Object* obj, std::string_view key)
{
    if (obj == nullptr) return nullptr;
    for (const auto& [name, value] : *obj) {
        if (name == key) return &value;
    }
    return nullptr;
}

[[nodiscard]] inline std::string last_segment(std::string_view path)
{
    auto bare = path.substr(0, path.find('?'));
    while (bare.size() > 1 && bare.back() == '/') bare.remove_suffix(1);
    const auto slash = bare.find_last_of('/');
    return std::string{slash == std::string_view::npos ? bare : bare.substr(slash + 1)};
}

/** @brief The store keys comments by numeric id; Ghost addresses them by wire id. */
[[nodiscard]] inline std::string store_id(std::string_view id)
{
    try {
        std::size_t used{0};
        (void)std::stoll(std::string{id}, &used);
        if (used == id.size()) return std::string{id};
    } catch (...) {
    }
    for (std::int64_t n = 0; n < 100000; ++n) {
        if (serde::ghost_id(n) == id) return std::to_string(n);
    }
    return std::string{id};
}

[[nodiscard]] inline std::string now_iso()
{
    const auto now = std::chrono::system_clock::now();
    return std::format("{:%Y-%m-%dT%H:%M:%S}Z", now);
}

} // namespace detail

class ExecuteStep {
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"comments_edit_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        const auto id = detail::last_segment(in.get_path());
        if (id.empty()) {
            return error("400", "Validation error", "Field value is required", "id");
        }

        std::string raw{in.body_as_string()};
        if (raw.empty()) raw = "{}";
        const auto decoded = core_serde::JsonCodec{}.decode<serde::Value>(raw);
        const auto* root = decoded ? std::get_if<detail::Object>(&decoded->get()) : nullptr;
        const detail::Object* data = root;
        detail::Object unwrapped;
        if (root != nullptr) {
            const auto* holder = detail::value_for(root, "data");
            const auto* data_obj = holder == nullptr
                                       ? nullptr
                                       : std::get_if<detail::Object>(&holder->get());
            if (data_obj != nullptr) {
                const auto* comments = detail::value_for(data_obj, "comments");
                const auto* array = comments == nullptr
                                        ? nullptr
                                        : std::get_if<serde::Value::Array>(&comments->get());
                if (array != nullptr && !array->empty()) {
                    if (const auto* first = std::get_if<detail::Object>(&array->front().get())) {
                        unwrapped = *first;
                        data = &unwrapped;
                    }
                }
            }
        }

        const auto status = detail::value_for(data, "status");
        const auto pinned = detail::value_for(data, "pinned");
        const bool has_status = status != nullptr;
        const bool has_pinned = pinned != nullptr;
        if (!has_status && !has_pinned) {
            return core::workflow::StepResponse<Output>{{{"status", "200"},
                {"body", local_mapping::wrap("comments", "[]")},
                {"invalidate", "/api/members/comments/" + id + "/"}}};
        }

        // Ghost moderateComment: only status and pinned are read; html is ignored.
        const std::string status_text = has_status
                                            ? core_serde::JsonCodec{}.encode(*status)
                                            : std::string{};
        const bool status_deleted = has_status && detail::unquote(status_text) == "deleted";
        bool pin_value{false};
        if (has_pinned) {
            const auto encoded = core_serde::JsonCodec{}.encode(*pinned);
            if (encoded != "true" && encoded != "false") {
                return error("400", "Pinned must be a boolean value", "Bad Request", "pinned");
            }
            pin_value = encoded == "true";
        }

        const auto records = blog::api::parse_array(ctx.store().json_for("comments"));
        const blog::api::RecordDto* existing = nullptr;
        for (const auto& record : records) {
            const auto it = record.find("id");
            if (it != record.end() && detail::unquote(it->second) == id) {
                existing = &record;
                break;
            }
        }
        if (has_pinned && pin_value) {
            if (existing == nullptr) {
                return core::workflow::StepResponse<Output>{{{"status", "404"},
                    {"body", blog::api::ApiError{}.json("Resource not found", "Not Found")}}};
            }
            const auto parent = existing->find("parent_id");
            const auto self = existing->find("id");
            const std::string parent_value = parent == existing->end()
                                                 ? std::string{}
                                                 : detail::unquote(parent->second);
            const std::string self_id = self == existing->end()
                                            ? std::string{}
                                            : detail::unquote(self->second);
            const bool is_reply = !parent_value.empty() && parent_value != "null" &&
                                  parent_value != self_id;
            if (is_reply) {
                return error("400", "Cannot pin a reply", "Bad Request", "pinned");
            }
            const auto current_status = existing->find("status");
            if (status_deleted || (current_status != existing->end() &&
                                   detail::unquote(current_status->second) == "deleted")) {
                return error("400", "Cannot pin a deleted comment", "Bad Request", "pinned");
            }
        }

        detail::Object updates;
        if (has_status) updates["status"] = *status;
        if (has_pinned) {
            if (status_deleted || !pin_value) {
                updates["pinned_at"] = serde::Value{}; // Ghost sets pinned_at = null
            } else {
                updates["pinned_at"] = serde::Value{detail::now_iso()};
            }
        }
        const auto body = core_serde::JsonCodec{}.encode(serde::Value{updates});

        const auto result = ctx.store().edit("comments", detail::store_id(id), body);
        if (!result.ok()) {
            return core::workflow::StepResponse<Output>{{{"status", std::to_string(result.status())},
                {"body", result.body()}}};
        }

        // Ghost adminEdit returns 200, not the store wrapper's 201.
        std::string wire;
        for (const auto& record : blog::api::parse_array(ctx.store().json_for("comments"))) {
            const auto it = record.find("id");
            if (it == record.end() || detail::unquote(it->second) != id) continue;
            wire = blog::api::record_to_json(record);
            break;
        }
        if (wire.empty()) wire = "{}";
        return core::workflow::StepResponse<Output>{{{"status", "200"},
            {"body", local_mapping::wrap("comments", local_mapping::clean(wire, "comment"))},
            {"invalidate", "/api/members/comments/" + id + "/"}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
    static blog::BlogContext* s_ctx;
private:
    static core::workflow::StepResponse<Output> error(std::string_view code,
                                                      std::string_view message,
                                                      std::string_view context,
                                                      std::string_view property)
    {
        return core::workflow::StepResponse<Output>{{{"status", std::string{code}},
            {"body", blog::api::ApiError{}.validation(message, context, property)}}};
    }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::comments::edit::workflow
