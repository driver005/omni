export module blog_handlers:labels_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import :outbox_shim;
import interfaces;

// ---------------------------------------------------------------------------
// Shared label support. Lives in this (already centrally registered) partition
// so no new partition / BUILD entry is required. Every other labels step
// imports :labels_browse_step to reuse it.
// ---------------------------------------------------------------------------
export namespace blog::handlers::labels::support {

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

inline constexpr std::string_view kDocName{"labels"};

[[nodiscard]] inline std::string quote(std::string_view value)
{
    std::string out{"\""};
    for (const unsigned char c: value) {
        switch (c) {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                if (c < 0x20) {
                    out += std::format("\\u{:04x}", static_cast<unsigned>(c));
                } else {
                    out.push_back(static_cast<char>(c));
                }
        }
    }
    out.push_back('"');
    return out;
}

[[nodiscard]] inline std::string unquote(std::string_view value)
{
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
        value.remove_suffix(1);
    }
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        value = value.substr(1, value.size() - 2);
    }
    std::string out;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size()) {
            ++i;
        }
        out.push_back(value[i]);
    }
    return out;
}

[[nodiscard]] inline std::size_t value_end(std::string_view text, std::size_t start)
{
    if (start >= text.size()) {
        return start;
    }
    const char first = text[start];
    if (first == '"') {
        bool escaped = false;
        for (std::size_t i = start + 1; i < text.size(); ++i) {
            if (escaped) {
                escaped = false;
                continue;
            }
            if (text[i] == '\\') {
                escaped = true;
                continue;
            }
            if (text[i] == '"') {
                return i + 1;
            }
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
                if (escaped) {
                    escaped = false;
                } else if (c == '\\') {
                    escaped = true;
                } else if (c == '"') {
                    in_string = false;
                }
                continue;
            }
            if (c == '"') {
                in_string = true;
            } else if (c == open) {
                ++depth;
            } else if (c == close && --depth == 0) {
                return i + 1;
            }
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
    if (at == std::string_view::npos) {
        return {};
    }
    const auto start = at + pattern.size();
    return std::string{object.substr(start, value_end(object, start) - start)};
}

[[nodiscard]] inline std::vector<std::string> objects(std::string_view array)
{
    std::vector<std::string> out;
    std::size_t i = 0;
    while (i < array.size()) {
        while (i < array.size() && array[i] != '{') {
            ++i;
        }
        if (i >= array.size()) {
            break;
        }
        const auto end = value_end(array, i);
        out.push_back(std::string{array.substr(i, end - i)});
        i = end;
    }
    return out;
}

[[nodiscard]] inline std::string wire_id(std::string_view stored)
{
    try {
        std::size_t used{0};
        const std::int64_t n = std::stoll(std::string{stored}, &used);
        if (used == stored.size()) {
            return serde::ghost_id(n);
        }
    } catch (...) {
    }
    return std::string{stored};
}

[[nodiscard]] inline std::string numeric_id(std::string_view id)
{
    try {
        std::size_t used{0};
        (void)std::stoll(std::string{id}, &used);
        if (used == id.size()) {
            return std::string{id};
        }
    } catch (...) {
    }
    for (std::int64_t n = 0; n < 100'000; ++n) {
        if (serde::ghost_id(n) == id) {
            return std::to_string(n);
        }
    }
    return std::string{id};
}

[[nodiscard]] inline std::string now()
{
    const auto tp = std::chrono::system_clock::now();
    const auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
    const auto stamp =
        std::format("{:%Y-%m-%dT%H:%M:%S}", std::chrono::floor<std::chrono::seconds>(tp));
    return stamp + std::format(".{:03d}Z", ms % 1'000);
}

/** @brief Rebuild one stored label into Ghost's serialized field set. */
[[nodiscard]] inline std::string
canonical(std::string_view object, std::string_view members_count = {})
{
    const auto id = wire_id(field(object, "id"));
    const auto name = field(object, "name");
    const auto slug = field(object, "slug");
    const auto created = field(object, "created_at");
    const auto updated = field(object, "updated_at");
    std::string count{R"({"posts":0})"};
    if (!members_count.empty()) {
        count = "{\"posts\":0,\"members\":" + std::string{members_count} + "}";
    }
    const std::string default_time = quote("1970-01-01T00:00:00.000Z");
    return "{\"id\":" + quote(id) + ",\"name\":" + (name.empty() ? quote("") : name) +
           ",\"slug\":" + (slug.empty() ? quote("") : slug) + ",\"count\":" + count +
           ",\"created_at\":" + (created.empty() ? default_time : created) +
           ",\"updated_at\":" + (updated.empty() ? default_time : updated) + "}";
}

[[nodiscard]] inline std::size_t members_for(blog::BlogContext& ctx, std::string_view stored_id)
{
    std::size_t total{0};
    for (const auto& link: objects(ctx.store().resources().list("members_labels"))) {
        if (unquote(field(link, "label_id")) == unquote(stored_id)) {
            ++total;
        }
    }
    return total;
}

/** @brief All persisted labels, serialized with Ghost's wire shape. */
[[nodiscard]] inline std::string rows(blog::BlogContext& ctx, bool with_members)
{
    const auto raw = ctx.store().resources().list("labels");
    std::string out{"["};
    bool first{true};
    for (const auto& object: objects(raw)) {
        if (!first) {
            out.push_back(',');
        }
        first = false;
        if (with_members) {
            out += canonical(object, std::to_string(members_for(ctx, field(object, "id"))));
        } else {
            out += canonical(object);
        }
    }
    out.push_back(']');
    return out;
}

[[nodiscard]] inline const serde::Value*
lookup(const serde::Value::Object& object, std::string_view key)
{
    for (const auto& [name, value]: object) {
        if (name == key) {
            return &value;
        }
    }
    return nullptr;
}

[[nodiscard]] inline std::string scalar(const serde::Value::Object& object, std::string_view key)
{
    const auto* value = lookup(object, key);
    if (value == nullptr) {
        return {};
    }
    if (const auto* text = std::get_if<std::string>(&value->get())) {
        return *text;
    }
    return {};
}

[[nodiscard]] inline bool has(const serde::Value::Object& object, std::string_view key)
{
    return lookup(object, key) != nullptr;
}

/** @brief Unwrap Ghost's `data.labels[0]` request envelope. */
[[nodiscard]] inline std::string unwrap(std::string_view body)
{
    if (body.empty()) {
        return "{}";
    }
    const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(body);
    if (!parsed) {
        return std::string{body};
    }
    const auto* root = std::get_if<serde::Value::Object>(&parsed->get());
    if (root == nullptr) {
        return std::string{body};
    }
    const auto* data = lookup(*root, "data");
    if (data != nullptr) {
        const auto* data_object = std::get_if<serde::Value::Object>(&data->get());
        if (data_object != nullptr) {
            const auto* labels = lookup(*data_object, "labels");
            if (labels != nullptr) {
                const auto* array = std::get_if<serde::Value::Array>(&labels->get());
                if (array != nullptr && !array->empty()) {
                    return core_serde::JsonCodec{}.encode((*array)[0]);
                }
            }
        }
    }
    const auto* labels = lookup(*root, "labels");
    if (labels != nullptr) {
        const auto* array = std::get_if<serde::Value::Array>(&labels->get());
        if (array != nullptr && !array->empty()) {
            return core_serde::JsonCodec{}.encode((*array)[0]);
        }
    }
    return std::string{body};
}

[[nodiscard]] inline std::string slugify(std::string_view name)
{
    std::string out;
    bool pending_dash{false};
    for (const unsigned char c: name) {
        if (std::isalnum(c)) {
            if (pending_dash && !out.empty()) {
                out.push_back('-');
            }
            pending_dash = false;
            out.push_back(static_cast<char>(std::tolower(c)));
        } else {
            pending_dash = true;
        }
    }
    while (!out.empty() && out.back() == '-') {
        out.pop_back();
    }
    return out;
}

[[nodiscard]] inline std::string
unique_slug(blog::BlogContext& ctx, std::string_view base, std::string_view except_id = {})
{
    std::set<std::string> used;
    for (const auto& object: objects(rows(ctx, false))) {
        const auto id = wire_id(field(object, "id"));
        if (!except_id.empty() && id == except_id) {
            continue;
        }
        const auto slug = unquote(field(object, "slug"));
        if (!slug.empty()) {
            used.insert(slug);
        }
    }
    std::string candidate{base};
    if (candidate.empty()) {
        candidate = "label";
    }
    std::size_t suffix{2};
    while (used.contains(candidate)) {
        candidate = std::string{base} + "-" + std::to_string(suffix++);
    }
    return candidate;
}

[[nodiscard]] inline bool
name_taken(blog::BlogContext& ctx, std::string_view name, std::string_view except_id = {})
{
    std::string wanted{name};
    std::ranges::transform(
        wanted,
        wanted.begin(),
        [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        }
    );
    for (const auto& object: objects(rows(ctx, false))) {
        const auto id = wire_id(field(object, "id"));
        if (!except_id.empty() && id == except_id) {
            continue;
        }
        std::string other = unquote(field(object, "name"));
        std::ranges::transform(
            other,
            other.begin(),
            [](unsigned char c)
            {
                return static_cast<char>(std::tolower(c));
            }
        );
        if (other == wanted) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] inline std::string
location(const interfaces::io::IRequest& request, std::string_view id)
{
    if (id.empty()) {
        return {};
    }
    const std::string host{request.find_header("host")};
    if (!host.empty()) {
        return "https://" + host + "/ghost/api/admin/labels/" + std::string{id} + "/";
    }
    std::string base{request.get_path()};
    if (const auto q = base.find('?'); q != std::string::npos) {
        base.erase(q);
    }
    while (base.size() > 1 && base.back() == '/') {
        base.pop_back();
    }
    return base + "/" + std::string{id} + "/";
}

[[nodiscard]] inline std::string
validation(std::string_view message, std::string_view context, std::string_view property)
{
    return blog::api::ApiError{}.validation(message, context, property, "ValidationError");
}

[[nodiscard]] inline std::string not_found()
{
    return blog::api::ApiError{}.validation("Label not found.", {}, {}, "NotFoundError");
}

[[nodiscard]] inline std::string bad_request(std::string_view message)
{
    return blog::api::ApiError{}.validation(message, {}, {}, "BadRequestError");
}

/** @brief Restricted-field guard shared by browse and both read routes. */
[[nodiscard]] inline bool restricted(const core::query::ParsedQuery& q)
{
    if (core::query::has_restricted_field(q.raw_filter) ||
        core::query::has_restricted_field(q.order)) {
        return true;
    }
    for (const auto& [name, value]: q.filters) {
        if (core::query::has_restricted_field(name) || core::query::has_restricted_field(value)) {
            return true;
        }
    }
    return false;
}

} // namespace blog::handlers::labels::support

export namespace blog::handlers::labels::browse::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"labels_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        auto& ctx = *s_ctx;

        constexpr std::array<std::string_view, 6>
            kOptions{"include", "filter", "fields", "limit", "order", "page"};
        auto q = core::query::parse(in.get_path(), kOptions);
        for (const auto& include: q.include) {
            if (include != "count.members") {
                return core::workflow::StepResponse<Output>{
                    {{"status", "422"},
                     {"body",
                      validation(
                          "Validation error, cannot browse labels.",
                          "Invalid include",
                          "include"
                      )}}
                };
            }
        }
        if (restricted(q)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body", bad_request("Restricted fields cannot be used in order.")}}
            };
        }
        // Ghost model/label.js orderDefaultOptions: name ASC, created_at DESC.
        if (q.order.empty()) {
            q.order = "name asc,created_at desc";
        }

        const auto available =
            local_mapping::clean(rows(ctx, q.include.contains("count.members")), "label");

        auto all_q = q;
        all_q.limit = std::numeric_limits<std::size_t>::max();
        all_q.page = 1;
        all_q.offset = 0;
        std::string filtered;
        std::string page_rows;
        try {
            filtered = core::query::apply(available, all_q);
            page_rows = core::query::apply(available, q);
        } catch (const std::runtime_error&) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body", bad_request("Restricted fields cannot be used in order.")}}
            };
        }

        const std::size_t total = objects(filtered).size();
        const std::size_t limit = q.limit == std::numeric_limits<std::size_t>::max()
                                      ? total
                                      : (q.limit == 0 ? std::size_t{15} : q.limit);
        const std::size_t page = q.page == 0 ? std::size_t{1} : q.page;
        const std::size_t pages = limit == 0 ? std::size_t{0} : (total + limit - 1) / limit;
        const std::string prev = page > 1 ? std::to_string(page - 1) : std::string{"null"};
        const std::string next = page < pages ? std::to_string(page + 1) : std::string{"null"};
        const std::string meta =
            "{\"pagination\":" + local_mapping::pagination(total, limit, page, pages, prev, next) +
            "}";
        const std::string body = local_mapping::wrap("labels", page_rows, meta);
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::labels::browse::workflow
