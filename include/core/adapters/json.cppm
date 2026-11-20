export module core_adapters:json;

import std;

export namespace core::adapters {

/** @brief JSON string escaping (quotes included). std-only, no serde dependency. */
[[nodiscard]] inline std::string quote(std::string_view text)
{
    std::string out;
    out.reserve(text.size() + 2);
    out.push_back('"');
    static constexpr char hex[] = "0123456789abcdef";
    for (const unsigned char character: text) {
        switch (character) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (character < 0x20) {
                out += "\\u00";
                out.push_back(hex[(character >> 4) & 0xF]);
                out.push_back(hex[character & 0xF]);
            } else {
                out.push_back(static_cast<char>(character));
            }
        }
    }
    out.push_back('"');
    return out;
}

/** @brief Ghost error envelope: {"errors":[{message,type,code?,context?}]}. */
[[nodiscard]] inline std::string ghost_error(
    std::string_view type,
    std::string_view message,
    std::string_view code = {},
    std::string_view context = {}
)
{
    std::string out = R"({"errors":[{"message":)";
    out += quote(message);
    out += R"(,"type":)";
    out += quote(type);
    if (!code.empty()) {
        out += R"(,"code":)";
        out += quote(code);
    }
    if (!context.empty()) {
        out += R"(,"context":)";
        out += quote(context);
    }
    out += "}]}";
    return out;
}

/** @brief True when text looks like a complete JSON object/array (balanced, string-aware). */
[[nodiscard]] inline bool valid_json(std::string_view text)
{
    std::size_t index = 0;
    while (index < text.size() && std::isspace(static_cast<unsigned char>(text[index]))) {
        ++index;
    }
    if (index >= text.size() || (text[index] != '{' && text[index] != '[')) {
        return false;
    }
    int depth = 0;
    bool in_string = false;
    bool escaped = false;
    for (; index < text.size(); ++index) {
        const char character = text[index];
        if (in_string) {
            if (escaped) {
                escaped = false;
            } else if (character == '\\') {
                escaped = true;
            } else if (character == '"') {
                in_string = false;
            }
            continue;
        }
        switch (character) {
        case '"': in_string = true; break;
        case '{':
        case '[': ++depth; break;
        case '}':
        case ']':
            if (--depth < 0) {
                return false;
            }
            break;
        default: break;
        }
    }
    return depth == 0 && !in_string;
}

/** @brief Percent-encoding for query values (RFC 3986 unreserved set kept verbatim). */
[[nodiscard]] inline std::string url_encode(std::string_view text)
{
    std::string out;
    out.reserve(text.size());
    static constexpr char hex[] = "0123456789ABCDEF";
    for (const unsigned char character: text) {
        const bool unreserved =
            (character >= 'a' && character <= 'z') ||
            (character >= 'A' && character <= 'Z') ||
            (character >= '0' && character <= '9') ||
            character == '-' || character == '_' || character == '.' || character == '~';
        if (unreserved) {
            out.push_back(static_cast<char>(character));
        } else {
            out.push_back('%');
            out.push_back(hex[(character >> 4) & 0xF]);
            out.push_back(hex[character & 0xF]);
        }
    }
    return out;
}

} // namespace core::adapters
