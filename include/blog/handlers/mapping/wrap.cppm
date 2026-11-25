// wrap.cppm — response envelopes and pagination for blog_handlers:mapping.
//
// Mirrors Ghost's output/default.js serializer:
//   frame.response[docName] = [...]; frame.response.meta = response.meta;
// and the `meta.pagination` object built by crud.js / memberBREADService.
export module blog_handlers:wrap;

import std;

export namespace blog::mapping {

/** @brief Wrap serialized rows in a Ghost collection envelope.
 *
 *  `rows_json` may be a JSON array (used as-is) or a single JSON object
 *  (wrapped in a one-element array).  Invalid/empty input yields `[]`.
 *  When `meta_json` is empty no `meta` key is emitted — matching Ghost's
 *  single-resource / create responses.  Otherwise the result is
 *  `{"<resource>":[<rows>],"meta":<meta_json>}`.
 */
[[nodiscard]] std::string
wrap(std::string_view resource, std::string_view rows_json, std::string_view meta_json = {});

/** @brief Build the `meta.pagination` object Ghost returns on browse routes.
 *
 *  `prev` / `next` are emitted as JSON `null` when empty or the literal string
 *  `"null"` (the convention used by the existing C++ handlers).  Otherwise an
 *  already-quoted JSON string is passed through; a bare value is quoted.
 */
[[nodiscard]] std::string pagination(
    std::size_t total,
    std::size_t limit,
    std::size_t page,
    std::size_t pages,
    std::string_view prev,
    std::string_view next
);

} // namespace blog::mapping

namespace blog::mapping::wrap_detail {

[[nodiscard]] inline std::string_view trim(std::string_view s)
{
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) {
        s.remove_prefix(1);
    }
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
        s.remove_suffix(1);
    }
    return s;
}

[[nodiscard]] inline std::string quote(std::string_view s)
{
    std::string out = "\"";
    for (const char c: s) {
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
                out += c;
        }
    }
    out += '"';
    return out;
}

[[nodiscard]] inline std::string link_field(std::string_view value)
{
    const auto v = trim(value);
    if (v.empty() || v == "null") {
        return "null";
    }
    if (v.front() == '"' && v.back() == '"' && v.size() >= 2) {
        return std::string{v};
    }
    return quote(v);
}

} // namespace blog::mapping::wrap_detail

namespace blog::mapping {

[[nodiscard]] std::string
wrap(std::string_view resource, std::string_view rows_json, std::string_view meta_json)
{
    using namespace wrap_detail;

    const auto rows = trim(rows_json);
    std::string body;
    if (rows.size() >= 2 && rows.front() == '[' && rows.back() == ']') {
        body = std::string{rows};
    } else if (rows.size() >= 2 && rows.front() == '{' && rows.back() == '}') {
        body = "[" + std::string{rows} + "]";
    } else {
        body = "[]";
    }

    std::string out = "{" + quote(resource) + ":" + body;
    const auto meta = trim(meta_json);
    if (!meta.empty()) {
        out += ",\"meta\":" + std::string{meta};
    }
    out += '}';
    return out;
}

[[nodiscard]] std::string pagination(
    std::size_t total,
    std::size_t limit,
    std::size_t page,
    std::size_t pages,
    std::string_view prev,
    std::string_view next
)
{
    using namespace wrap_detail;
    std::string out = "{\"total\":" + std::to_string(total) +
                      ",\"limit\":" + std::to_string(limit) + ",\"page\":" + std::to_string(page) +
                      ",\"pages\":" + std::to_string(pages) + ",\"prev\":" + link_field(prev) +
                      ",\"next\":" + link_field(next) + "}";
    return out;
}

} // namespace blog::mapping
