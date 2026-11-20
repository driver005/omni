export module blog_handlers:feedback_read_by_id_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import interfaces;
import blog_handlers_middleware;

// ---------------------------------------------------------------------------
// Shared feedback support. This partition is already centrally registered, so
// the add step imports these helpers instead of duplicating them (no new
// partition / BUILD / base.cppm edit is required).
// ---------------------------------------------------------------------------
export namespace blog::handlers::feedback::support {

inline constexpr std::string_view kFeedBackResource{"members_feedback"};
inline constexpr std::string_view kWithMemberHeader{"x-blog-with-member"};
inline constexpr std::string_view kMemberIdHeader{"X-Member-Id"};

/** @brief JSON string literal with Ghost's escaping rules. */
[[nodiscard]] inline std::string quote(std::string_view value)
{
    std::string out{"\""};
    for (const unsigned char c : value) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) out += std::format("\\u{:04x}", static_cast<unsigned>(c));
                else out.push_back(static_cast<char>(c));
        }
    }
    out.push_back('"');
    return out;
}

[[nodiscard]] inline std::string_view trim(std::string_view value)
{
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
        value.remove_suffix(1);
    }
    return value;
}

/** @brief Unquote and unescape a raw JSON string value. */
[[nodiscard]] inline std::string unquote(std::string_view value)
{
    value = trim(value);
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        value = value.substr(1, value.size() - 2);
    }
    std::string out;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size()) {
            ++i;
            switch (value[i]) {
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                default: out.push_back(value[i]); break;
            }
        } else {
            out.push_back(value[i]);
        }
    }
    return out;
}

/** @brief Index just past one raw JSON value (string, object or scalar). */
[[nodiscard]] inline std::size_t value_end(std::string_view text, std::size_t start)
{
    if (start >= text.size()) return start;
    const char first = text[start];
    if (first == '"') {
        bool escaped = false;
        for (std::size_t i = start + 1; i < text.size(); ++i) {
            if (escaped) { escaped = false; continue; }
            if (text[i] == '\\') { escaped = true; continue; }
            if (text[i] == '"') return i + 1;
        }
        return text.size();
    }
    if (first == '{' || first == '[') {
        const char open = first;
        const char close = first == '{' ? '}' : ']';
        int depth = 0;
        bool in_string = false;
        bool escaped = false;
        for (std::size_t i = start; i < text.size(); ++i) {
            const char c = text[i];
            if (in_string) {
                if (escaped) escaped = false;
                else if (c == '\\') escaped = true;
                else if (c == '"') in_string = false;
                continue;
            }
            if (c == '"') in_string = true;
            else if (c == open) ++depth;
            else if (c == close && --depth == 0) return i + 1;
        }
        return text.size();
    }
    std::size_t i = start;
    while (i < text.size() && text[i] != ',' && text[i] != '}' && text[i] != ']' &&
           !std::isspace(static_cast<unsigned char>(text[i]))) {
        ++i;
    }
    return i;
}

/** @brief Raw JSON value fragment of @p key in a flat object (quotes kept). */
[[nodiscard]] inline std::string field(std::string_view object, std::string_view key)
{
    const auto pattern = std::string{"\""} + std::string{key} + "\":";
    const auto at = object.find(pattern);
    if (at == std::string_view::npos) return {};
    const auto start = at + pattern.size();
    return std::string{object.substr(start, value_end(object, start) - start)};
}

[[nodiscard]] inline std::string text_field(std::string_view object, std::string_view key)
{
    return unquote(field(object, key));
}

/** @brief Ghost-shaped error envelope (type defaults to ValidationError). */
[[nodiscard]] inline std::string error(std::string_view message, std::string_view context,
                                       std::string_view property,
                                       std::string_view type = "ValidationError")
{
    return blog::api::ApiError{}.validation(message, context, property, type);
}

/** @brief Last path segment (the `:id` bound by the admin route). */
[[nodiscard]] inline std::string last_segment(std::string_view path)
{
    path = path.substr(0, path.find('?'));
    while (path.size() > 1 && path.back() == '/') path.remove_suffix(1);
    const auto slash = path.find_last_of('/');
    return std::string{slash == std::string_view::npos ? path : path.substr(slash + 1)};
}

/** @brief Ghost `parseInt` semantics for the `score` option. */
[[nodiscard]] inline long long parse_int(std::string_view text)
{
    try {
        std::size_t used = 0;
        const long long value = std::stoll(std::string{text}, &used);
        return used == 0 ? 0 : value;
    } catch (...) {
        return 0;
    }
}

} // namespace blog::handlers::feedback::support

export namespace blog::handlers::feedback::read_by_id::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"feedback_read_by_id_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        auto& ctx = *s_ctx;

        const auto id = last_segment(in.get_path());
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{{{"status", "422"},
                {"body", error("Validation error", "An id is required", "id")}}};
        }

        // Ghost declares options ['limit','page','order','score']; `filter` is the
        // NQL input surface shared by the query pipeline.
        constexpr std::array<std::string_view, 5> kOptions{
            "limit", "page", "order", "score", "filter"};
        auto q = core::query::parse(in.get_path(), kOptions);

        // FeedbackRepository.getForPost always filters post_id and (when present)
        // score, and hard-codes order:'created_at DESC' — the user `order` option
        // is accepted by the controller but ignored downstream.
        q.filters.emplace_back("post_id", id);
        if (const auto it = q.options.find("score"); it != q.options.end() &&
                                                       !it->second.empty()) {
            q.filters.emplace_back("score", std::to_string(parse_int(it->second)));
        }
        q.order = "created_at desc";
        if (!q.options.contains("limit")) q.limit = 10;  // Ghost: limit || 10

        // Results are projected to the repository's wire fields. `post_id` is
        // kept in the source rows for filtering but dropped from the output.
        q.fields = {"id", "score", "created_at", "member"};

        const bool with_member = in.find_header(kWithMemberHeader) == "true";
        const auto stored = blog::api::parse_array(
            ctx.store().json_for(kFeedBackResource, {}));

        std::string rows{"["};
        bool first{true};
        for (const auto& entry : stored) {
            const auto record = blog::api::record_to_json(entry);
            if (!first) rows.push_back(',');
            first = false;
            std::string object{"{"};
            object += "\"id\":" + (field(record, "id").empty()
                                       ? quote(id) : field(record, "id"));
            object += ",\"score\":" + (field(record, "score").empty()
                                          ? std::string{"0"} : field(record, "score"));
            object += ",\"created_at\":" + (field(record, "created_at").empty()
                                               ? quote("") : field(record, "created_at"));
            object += ",\"post_id\":" + (field(record, "post_id").empty()
                                            ? quote("") : field(record, "post_id"));
            if (with_member) {
                const auto member_id = text_field(record, "member_id");
                if (!member_id.empty()) {
                    if (const auto member = ctx.store().find_by_resource("members", member_id)) {
                        object += ",\"member\":" + *member;
                    }
                }
            }
            object.push_back('}');
            rows += object;
        }
        rows.push_back(']');

        std::string body;
        try {
            // core::query::run applies NQL filter/sort/paging and builds Ghost's
            // meta.pagination object, replacing hand-rolled pagination.
            body = core::query::run(rows, q, 10);
        } catch (const std::runtime_error&) {
            return core::workflow::StepResponse<Output>{{{"status", "400"},
                {"body", error("Restricted fields cannot be used in order.",
                               "Restricted fields cannot be used in order.", "")}}};
        }
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::feedback::read_by_id::workflow
