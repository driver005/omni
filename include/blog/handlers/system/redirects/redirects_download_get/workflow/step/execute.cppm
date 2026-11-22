export module blog_handlers:redirects_redirects_download_get_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import core_serde;

export namespace blog::handlers::redirects::redirects_download_get::workflow {

namespace download_detail {

// Ghost `serializeToYaml` dumps every pair with js-yaml
// (`yaml.dump({[from]: to}, {lineWidth:-1})`, redirect-config-parser.ts:138-150).
// These helpers reproduce js-yaml 4's scalar style choice for the string keys
// and string values that make up a redirect pair. No third-party lib is used.

struct Codepoint { unsigned value{}; std::size_t len{}; };

[[nodiscard]] inline Codepoint decode(std::string_view s, std::size_t i) noexcept
{
    const auto b = static_cast<unsigned char>(s[i]);
    if (b < 0x80) return {b, 1};
    if ((b & 0xE0) == 0xC0 && i + 1 < s.size()) {
        return {static_cast<unsigned>(((b & 0x1F) << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3F)), 2};
    }
    if ((b & 0xF0) == 0xE0 && i + 2 < s.size()) {
        return {static_cast<unsigned>(((b & 0x0F) << 12) | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 6) |
                    (static_cast<unsigned char>(s[i + 2]) & 0x3F)),
                3};
    }
    if ((b & 0xF8) == 0xF0 && i + 3 < s.size()) {
        return {static_cast<unsigned>(((b & 0x07) << 18) | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 12) |
                    ((static_cast<unsigned char>(s[i + 2]) & 0x3F) << 6) |
                    (static_cast<unsigned char>(s[i + 3]) & 0x3F)),
                4};
    }
    return {b, 1};
}

[[nodiscard]] inline bool is_printable(unsigned c) noexcept
{
    return (c >= 0x20 && c <= 0x7E) || (c >= 0xA1 && c <= 0xD7FF && c != 0x2028 && c != 0x2029) ||
           (c >= 0xE000 && c <= 0xFFFD) || (c >= 0x10000 && c <= 0x10FFFF);
}

[[nodiscard]] inline bool is_whitespace(unsigned c) noexcept { return c == 0x20 || c == 0x09; }
[[nodiscard]] inline bool is_ns_char_or_ws(unsigned c) noexcept
{
    return is_printable(c) && c != 0xFEFF && c != '\r' && c != '\n';
}

[[nodiscard]] inline bool is_plain_safe_first(unsigned c) noexcept
{
    if (!is_printable(c) || c == 0xFEFF || is_whitespace(c)) return false;
    switch (c) {
        case '-': case '?': case ':': case ',': case '[': case ']': case '{': case '}':
        case '#': case '&': case '*': case '!': case '|': case '=': case '>': case '\'':
        case '"': case '%': case '@': case '`':
            return false;
        default:
            return true;
    }
}

[[nodiscard]] inline bool is_plain_safe_last(unsigned c) noexcept
{
    return !is_whitespace(c) && c != ':';
}

[[nodiscard]] inline bool is_plain_safe(unsigned c, unsigned prev, bool inblock) noexcept
{
    const bool ns_or_ws = is_ns_char_or_ws(c);
    const bool ns = ns_or_ws && !is_whitespace(c);
    bool safe = inblock ? ns_or_ws
                        : ns_or_ws && c != ',' && c != '[' && c != ']' && c != '{' && c != '}';
    safe = safe && c != '#'; // false on '#'
    if (prev == ':' && !ns) safe = false; // false on ': '
    if (!safe && is_ns_char_or_ws(prev) && !is_whitespace(prev) && c == '#') safe = true;
    if (!safe && prev == ':' && ns) safe = true; // ':[^ ]'
    return safe;
}

[[nodiscard]] inline bool iequal(std::string_view a, std::string_view b) noexcept
{
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (static_cast<char>(std::tolower(static_cast<unsigned char>(a[i]))) !=
            static_cast<char>(std::tolower(static_cast<unsigned char>(b[i])))) return false;
    }
    return true;
}

[[nodiscard]] inline bool all_digits(std::string_view s) noexcept
{
    if (s.empty()) return false;
    for (const char c : s) if (c < '0' || c > '9') return false;
    return true;
}

// js-yaml's implicit resolvers + noCompatMode:default deprecated booleans.
[[nodiscard]] inline bool ambiguous(std::string_view s) noexcept
{
    if (s.empty()) return true; // null
    if (s == "~" || iequal(s, "null")) return true;
    if (iequal(s, "true") || iequal(s, "false")) return true;
    for (const std::string_view d : {"y", "yes", "on", "n", "no", "off"}) {
        if (iequal(s, d)) return true;
    }
    // .inf / .nan with optional sign
    std::string_view t = s;
    if (t.front() == '+' || t.front() == '-') t.remove_prefix(1);
    if (iequal(t, ".inf") || iequal(t, ".nan")) return true;

    // sexagesimal (base60) syntax always forces quotes in js-yaml
    if (t.find(':') != std::string_view::npos) {
        bool shape = true;
        for (const char c : t) {
            if (!((c >= '0' && c <= '9') || c == '_' || c == ':' || c == '.')) { shape = false; break; }
        }
        if (shape) return true;
    }

    // integers / floats: optional sign, digits/underscores, optional fraction and exponent,
    // plus 0b/0o/0x prefixed forms.
    t = s;
    if (!t.empty() && (t.front() == '+' || t.front() == '-')) t.remove_prefix(1);
    if (t.starts_with("0b") || t.starts_with("0o") || t.starts_with("0x")) t.remove_prefix(2);
    bool numeric = !t.empty();
    bool seen_digit = false;
    for (std::size_t i = 0; i < t.size(); ++i) {
        const char c = t[i];
        if (c >= '0' && c <= '9') { seen_digit = true; continue; }
        if (c == '_' || c == '.') continue;
        if (c == 'e' || c == 'E') {
            if (i + 1 < t.size() && (t[i + 1] == '+' || t[i + 1] == '-')) ++i;
            continue;
        }
        if ((c == 'a' || c == 'A' || c == 'b' || c == 'B' || c == 'c' || c == 'C' ||
             c == 'd' || c == 'D' || c == 'f' || c == 'F')) {
            if (s.find("0x") != std::string_view::npos || s.find("0X") != std::string_view::npos) continue;
        }
        numeric = false;
        break;
    }
    if (numeric && seen_digit) return true;

    // timestamp: YYYY-MM-DD (optionally followed by a time)
    if (s.size() >= 10 && all_digits(s.substr(0, 4)) && s[4] == '-' &&
        all_digits(s.substr(5, 2)) && s[7] == '-' && all_digits(s.substr(8, 2))) return true;
    return false;
}

enum class Style { plain, single, double_quoted, literal };

[[nodiscard]] inline Style choose_style(std::string_view s, bool iskey, bool inblock) noexcept
{
    if (s.empty()) return Style::single;
    bool plain = is_plain_safe_first(decode(s, 0).value) &&
                 is_plain_safe_last(decode(s, s.size() - 1).value);
    bool line_break = false;
    unsigned prev = 0;
    for (std::size_t i = 0; i < s.size();) {
        const auto cp = decode(s, i);
        if (cp.value == '\n') {
            if (iskey) return Style::double_quoted; // singleLineOnly: no block styles for keys
            line_break = true;
        } else if (!is_printable(cp.value)) {
            return Style::double_quoted;
        }
        plain = plain && is_plain_safe(cp.value, prev, inblock);
        prev = cp.value;
        i += cp.len;
    }
    if (!line_break) return plain && !ambiguous(s) ? Style::plain : Style::single;
    return Style::literal; // lineWidth:-1 disables folding; block literal wins for values
}

[[nodiscard]] inline std::string double_quoted(std::string_view s)
{
    static constexpr std::string_view kHex{"0123456789ABCDEF"};
    std::string out{"\""};
    for (std::size_t i = 0; i < s.size();) {
        const auto cp = decode(s, i);
        switch (cp.value) {
            case 0x00: out += "\\0"; break;
            case 0x07: out += "\\a"; break;
            case 0x08: out += "\\b"; break;
            case 0x09: out += "\\t"; break;
            case 0x0A: out += "\\n"; break;
            case 0x0B: out += "\\v"; break;
            case 0x0C: out += "\\f"; break;
            case 0x0D: out += "\\r"; break;
            case 0x1B: out += "\\e"; break;
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case 0x85: out += "\\N"; break;
            case 0xA0: out += "\\_"; break;
            case 0x2028: out += "\\L"; break;
            case 0x2029: out += "\\P"; break;
            default:
                if (is_printable(cp.value)) {
                    out.append(s.substr(i, cp.len));
                } else if (cp.value <= 0xFF) {
                    out += "\\x";
                    out += kHex[(cp.value >> 4) & 0xF];
                    out += kHex[cp.value & 0xF];
                } else if (cp.value <= 0xFFFF) {
                    out += "\\u";
                    for (int shift = 12; shift >= 0; shift -= 4) out += kHex[(cp.value >> shift) & 0xF];
                } else {
                    out += "\\U";
                    for (int shift = 28; shift >= 0; shift -= 4) out += kHex[(cp.value >> shift) & 0xF];
                }
                break;
        }
        i += cp.len;
    }
    out += '"';
    return out;
}

[[nodiscard]] inline std::string single_quoted(std::string_view s)
{
    std::string out{"'"};
    for (const char c : s) {
        out += c;
        if (c == '\'') out += '\'';
    }
    out += '\'';
    return out;
}

[[nodiscard]] inline std::string literal(std::string_view s, std::size_t indent)
{
    // needIndentIndicator: /^\n* /
    std::size_t lead = 0;
    while (lead < s.size() && s[lead] == '\n') ++lead;
    std::string out{"|"};
    if (lead < s.size() && s[lead] == ' ') out += std::to_string(indent);
    const bool clip = !s.empty() && s.back() == '\n';
    const bool keep = clip && (s.size() >= 2 && s[s.size() - 2] == '\n');
    out += keep ? "+" : (clip ? "" : "-");
    out += '\n';

    // indentString(): every non-empty line (including trailing '\n') is indented.
    std::size_t pos = 0;
    while (pos < s.size()) {
        const auto nl = s.find('\n', pos);
        const auto line = s.substr(pos, (nl == std::string_view::npos ? s.size() : nl + 1) - pos);
        if (!line.empty() && line != "\n") out.append(indent, ' ');
        out += line;
        pos = nl == std::string_view::npos ? s.size() : nl + 1;
    }
    if (!out.empty() && out.back() == '\n') out.pop_back(); // dropEndingNewline
    return out;
}

[[nodiscard]] inline std::string scalar(std::string_view s, bool iskey, bool inblock, std::size_t indent)
{
    switch (choose_style(s, iskey, inblock)) {
        case Style::plain: return std::string{s};
        case Style::single: return single_quoted(s);
        case Style::double_quoted: return double_quoted(s);
        case Style::literal: return literal(s, indent);
    }
    return {};
}

// Mirror js-yaml `yaml.dump({[from]: to}, {lineWidth:-1})` for one pair.
[[nodiscard]] inline std::string dump_pair(std::string_view from, std::string_view to)
{
    const auto key = scalar(from, true, true, 2);
    const auto value = scalar(to, false, true, 2);
    return key + (value.starts_with('\n') ? ":" : ": ") + value + "\n";
}

} // namespace download_detail

class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::string;
 static constexpr std::string_view name{"redirects_redirects_download_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  (void)in;
  auto& ctx=*s_ctx;

  // Ghost `serializeToYaml(redirects)`: group permanent rules under 301:, rest under 302:.
  const auto rows=ctx.store().json_for("redirects", {});
  std::string p301, p302;
  for (const auto& rec : blog::api::parse_array(rows)) {
   const auto get=[&](std::string_view k) {
    const auto it=rec.find(std::string{k});
    if (it==rec.end()) return std::string{};
    if (it->second.starts_with('"')) {
     if (auto s=core_serde::JsonCodec{}.decode<std::string>(it->second)) return *s;
    }
    return it->second;
   };
   const auto from=get("from"), to=get("to");
   if (from.empty() || to.empty()) continue;
   const auto pit=rec.find("permanent");
   const bool perm=pit!=rec.end() &&
       (pit->second=="true" || core_serde::JsonCodec{}.decode<bool>(pit->second).value_or(false));
   // formatSection(): section header by hand; pairs through yaml.dump; every
   // dumped (non-empty) line indented by two spaces; trailing newline per section.
   auto& bucket=perm?p301:p302;
   const auto dumped=download_detail::dump_pair(from,to);
   for (const auto line : std::views::split(dumped, '\n')) {
    if (line.empty()) continue;
    bucket += "  ";
    bucket += std::string_view{std::ranges::begin(line), std::ranges::end(line)};
    bucket += '\n';
   }
  }
  std::string yaml=(!p301.empty()?"301:\n"+p301:"")+(!p302.empty()?"302:\n"+p302:"");
  if (yaml.empty()) yaml="# See https://docs.ghost.org/themes/routing/#redirects\n301: {}\n302: {}\n";
  return core::workflow::StepResponse<Output>{yaml};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
