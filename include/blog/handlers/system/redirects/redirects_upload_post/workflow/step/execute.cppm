export module blog_handlers:redirects_redirects_upload_post_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import core_serde;

export namespace blog::handlers::redirects::redirects_upload_post::workflow {

namespace upload_detail {

    using Record = blog::api::RecordDto;

    inline constexpr std::string_view kHelp{"https://docs.ghost.org/themes/routing/#redirects"};

    [[nodiscard]] inline std::string trim_copy(std::string_view v)
    {
        while (!v.empty() && std::isspace(static_cast<unsigned char>(v.front()))) {
            v.remove_prefix(1);
        }
        while (!v.empty() && std::isspace(static_cast<unsigned char>(v.back()))) {
            v.remove_suffix(1);
        }
        return std::string{v};
    }

    [[nodiscard]] inline std::string quote(std::string_view s)
    {
        return core_serde::JsonCodec{}.encode(std::string{s});
    }

    // Ghost error envelopes: same field set as core_auth::ghost, with the help URL
    // Ghost includes on redirect parse/validation errors.
    [[nodiscard]] inline std::string
    ghost_error(std::string_view message, std::string_view type, std::string_view help = {})
    {
        static std::uint64_t counter{0};
        return std::format(
            R"({{"errors":[{{"id":"{:032x}","code":null,"context":null,"details":null,"ghostErrorCode":null,"help":{},"message":{},"property":null,"type":{}}}]}})",
            ++counter,
            help.empty() ? "null" : quote(help),
            quote(message),
            quote(type)
        );
    }

    [[nodiscard]] inline core::workflow::StepResponse<std::map<std::string, std::string>> fail(
        std::string_view status,
        std::string_view message,
        std::string_view type,
        std::string_view help = {}
    )
    {
        return core::workflow::StepResponse<std::map<std::string, std::string>>{
            {{"status", std::string{status}}, {"body", ghost_error(message, type, help)}}
        };
    }

    struct Part
    {
        bool found{false};
        std::string filename;
        std::string mimetype;
        std::string payload;
    };

    // Multer's `single('redirects')` populates req.file only for a multipart part
    // named "redirects" carrying a filename. Everything else is a missing file.
    [[nodiscard]] inline Part extract_part(std::string_view body, std::string_view content_type)
    {
        Part out;
        const auto bpos = content_type.find("boundary=");
        if (bpos == std::string_view::npos) {
            return out;
        }

        std::string boundary{trim_copy(content_type.substr(bpos + 9))};
        if (const auto semi = boundary.find(';'); semi != std::string::npos) {
            boundary.resize(semi);
        }
        if (boundary.size() >= 2 && boundary.front() == '"' && boundary.back() == '"') {
            boundary = boundary.substr(1, boundary.size() - 2);
        }
        if (boundary.empty()) {
            return out;
        }

        const std::string marker = "--" + boundary;
        std::size_t pos = 0;
        while ((pos = body.find(marker, pos)) != std::string_view::npos) {
            pos += marker.size();
            if (body.substr(pos, 2) == "--") {
                break; // terminal boundary
            }
            if (body.substr(pos, 2) == "\r\n") {
                pos += 2;
            } else if (body.substr(pos, 1) == "\n") {
                pos += 1;
            } else {
                continue;
            }

            std::size_t head_end = body.find("\r\n\r\n", pos), sep = 4;
            if (head_end == std::string_view::npos) {
                head_end = body.find("\n\n", pos);
                sep = 2;
            }
            if (head_end == std::string_view::npos) {
                break;
            }
            const auto headers = body.substr(pos, head_end - pos);
            const auto payload_start = head_end + sep;
            const auto next = body.find(marker, payload_start);
            auto payload = body.substr(
                payload_start,
                (next == std::string_view::npos ? body.size() : next) - payload_start
            );
            while (!payload.empty() && (payload.back() == '\r' || payload.back() == '\n')) {
                payload.remove_suffix(1);
            }

            if (headers.find("name=\"redirects\"") == std::string_view::npos) {
                pos = payload_start;
                continue;
            }

            const auto fpos = headers.find("filename=\"");
            if (fpos == std::string_view::npos) {
                pos = payload_start;
                continue;
            } // field, not a file
            const auto fend = headers.find('"', fpos + 10);
            if (fend == std::string_view::npos) {
                pos = payload_start;
                continue;
            }
            out.filename = std::string{headers.substr(fpos + 10, fend - (fpos + 10))};
            if (const auto cpos = headers.find("Content-Type:"); cpos != std::string_view::npos) {
                const auto line_end = headers.find("\r\n", cpos);
                auto value = headers.substr(
                    cpos + 13,
                    line_end == std::string_view::npos ? headers.size() - cpos - 13
                                                       : line_end - cpos - 13
                );
                out.mimetype = trim_copy(value);
            }
            if (out.mimetype.empty()) {
                out.mimetype = "application/octet-stream"; // busboy default
            }
            out.payload = std::string{payload};
            out.found = true;
            return out;
        }
        return out;
    }

    [[nodiscard]] inline bool allowed_extension(std::string_view ext) noexcept
    {
        // config/overrides.json uploads.redirects.extensions == [".json", ".yaml"]
        return ext == ".json" || ext == ".yaml";
    }

    [[nodiscard]] inline bool allowed_content_type(std::string_view ct) noexcept
    {
        // config/overrides.json uploads.redirects.contentTypes
        return ct == "text/plain" || ct == "text/yaml" || ct == "application/octet-stream" ||
               ct == "application/json" || ct == "application/yaml" || ct == "application/x-yaml";
    }

    [[nodiscard]] inline std::string extension_of(std::string_view name)
    {
        const auto slash = name.find_last_of("/\\");
        const auto base = slash == std::string_view::npos ? name : name.substr(slash + 1);
        const auto dot = base.find_last_of('.');
        if (dot == std::string_view::npos) {
            return {};
        }
        std::string ext{base.substr(dot)};
        for (char& c: ext) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return ext;
    }

    // js-yaml 4 core-schema resolution for the scalars Ghost rejects with
    // `typeof to !== 'string'` (redirect-config-parser.ts:80-90).
    [[nodiscard]] inline bool resolves_nonstring(std::string_view t) noexcept
    {
        if (t.empty() || t == "~" || t == "null" || t == "Null" || t == "NULL") {
            return true;
        }
        if (t == "true" || t == "True" || t == "TRUE" || t == "false" || t == "False" ||
            t == "FALSE") {
            return true;
        }
        // Timestamps resolve to Date objects.
        if (t.size() >= 10 && t[4] == '-' && t[7] == '-') {
            bool date = true;
            for (std::size_t i = 0; i < 10; ++i) {
                if (i == 4 || i == 7) {
                    continue;
                }
                if (t[i] < '0' || t[i] > '9') {
                    date = false;
                    break;
                }
            }
            if (date) {
                return true;
            }
        }
        std::string_view s = t;
        if (!s.empty() && (s.front() == '+' || s.front() == '-')) {
            s.remove_prefix(1);
        }
        if (s.empty()) {
            return false;
        }
        if (s == ".inf" || s == ".Inf" || s == ".INF" || s == ".nan" || s == ".NaN" ||
            s == ".NAN") {
            return true;
        }
        if (s.starts_with("0x") || s.starts_with("0X")) {
            s.remove_prefix(2);
            if (s.empty()) {
                return false;
            }
            for (const char c: s) {
                if (!std::isxdigit(static_cast<unsigned char>(c))) {
                    return false;
                }
            }
            return true;
        }
        if (s.starts_with("0o") || s.starts_with("0O")) {
            s.remove_prefix(2);
            if (s.empty()) {
                return false;
            }
            for (const char c: s) {
                if (c < '0' || c > '7') {
                    return false;
                }
            }
            return true;
        }
        if (s.starts_with("0b") || s.starts_with("0B")) {
            s.remove_prefix(2);
            if (s.empty()) {
                return false;
            }
            for (const char c: s) {
                if (c != '0' && c != '1') {
                    return false;
                }
            }
            return true;
        }
        bool digits = false, dot = false, exp = false;
        for (std::size_t i = 0; i < s.size(); ++i) {
            const char c = s[i];
            if (c >= '0' && c <= '9') {
                digits = true;
                continue;
            }
            if (c == '.' && !dot && !exp) {
                dot = true;
                continue;
            }
            if ((c == 'e' || c == 'E') && !exp && digits) {
                exp = true;
                if (i + 1 < s.size() && (s[i + 1] == '+' || s[i + 1] == '-')) {
                    ++i;
                }
                continue;
            }
            return false; // any other char -> plain string
        }
        return digits;
    }

    // Decode a YAML scalar (single-quoted, double-quoted, or plain). `check_type`
    // rejects plain scalars that js-yaml would coerce away from string (values only).
    [[nodiscard]] inline std::string yaml_scalar(std::string_view raw, bool& ok, bool check_type)
    {
        const auto trimmed = trim_copy(raw);
        std::string_view t{trimmed};
        if (t.size() >= 2 && t.front() == '\'' && t.back() == '\'') {
            std::string out;
            t.remove_prefix(1);
            t.remove_suffix(1);
            for (std::size_t i = 0; i < t.size(); ++i) {
                if (t[i] == '\'' && i + 1 < t.size() && t[i + 1] == '\'') {
                    out += '\'';
                    ++i;
                } else {
                    out += t[i];
                }
            }
            return out;
        }
        if (t.size() >= 2 && t.front() == '"' && t.back() == '"') {
            if (auto s = core_serde::JsonCodec{}.decode<std::string>(std::string{t})) {
                return *s;
            }
            ok = false;
            return {};
        }
        if (t.empty() || (check_type && resolves_nonstring(t))) {
            ok = false;
            return {};
        }
        return std::string{t};
    }

    [[nodiscard]] inline Record make_record(std::string from, std::string to, bool permanent)
    {
        Record rec;
        rec["from"] = quote(from);
        rec["to"] = quote(to);
        rec["permanent"] = permanent ? "true" : "false";
        return rec;
    }

    // Strict subset of js-yaml's `yaml.load` for the redirect config shape: a plain
    // mapping with own 301/302 sections, each a mapping of string -> string.
    [[nodiscard]] inline bool parse_yaml(std::string_view text, std::vector<Record>& out)
    {
        struct Raw
        {
            std::string_view text;
            std::size_t indent;
        };

        std::vector<Raw> lines;
        std::size_t begin = 0;
        while (begin <= text.size()) {
            const auto nl = text.find('\n', begin);
            auto line =
                text.substr(begin, (nl == std::string_view::npos ? text.size() : nl) - begin);
            begin = (nl == std::string_view::npos ? text.size() + 1 : nl + 1);
            if (!line.empty() && line.back() == '\r') {
                line.remove_suffix(1);
            }
            std::size_t indent = 0;
            while (indent < line.size() && (line[indent] == ' ' || line[indent] == '\t')) {
                ++indent;
            }
            lines.push_back({line.substr(indent), indent});
        }

        bool saw_section = false;
        int section = 0; // 301 or 302
        bool inline_empty = false;
        for (std::size_t i = 0; i < lines.size(); ++i) {
            auto line = lines[i].text;
            const auto indent = lines[i].indent;
            const auto trimmed = trim_copy(line);
            if (trimmed.empty() || trimmed.front() == '#' || trimmed == "---" || trimmed == "...") {
                continue;
            }

            if (indent == 0) {
                if (trimmed.starts_with("301:") || trimmed.starts_with("302:")) {
                    section = trimmed.starts_with("301:") ? 301 : 302;
                    saw_section = true;
                    const auto rest = trim_copy(trimmed.substr(trimmed.find(':') + 1));
                    // A bare `301:` opens a block mapping; `301: {}` is an explicit empty
                    // mapping that cannot have nested entries. Any other inline value makes
                    // the section a scalar/list and Ghost rejects it with BadRequestError.
                    inline_empty = (rest == "{}");
                    if (!rest.empty() && !inline_empty) {
                        return false;
                    }
                    continue;
                }
                return false; // scalar/list/unknown top-level key -> BadRequestError
            }

            if (!saw_section || inline_empty || trimmed.starts_with("- ")) {
                return false;
            }

            // Find the mapping colon outside quotes; it must be followed by space or EOL.
            std::size_t colon = std::string_view::npos;
            char quote_char = 0;
            for (std::size_t j = 0; j < trimmed.size(); ++j) {
                const char c = trimmed[j];
                if (quote_char) {
                    if (c == '\\' && quote_char == '"') {
                        ++j;
                        continue;
                    }
                    if (c == quote_char) {
                        if (quote_char == '\'' && j + 1 < trimmed.size() &&
                            trimmed[j + 1] == '\'') {
                            ++j;
                            continue;
                        }
                        quote_char = 0;
                    }
                    continue;
                }
                if (c == '\'' || c == '"') {
                    quote_char = c;
                    continue;
                }
                if (c == ':' && (j + 1 == trimmed.size() ||
                                 std::isspace(static_cast<unsigned char>(trimmed[j + 1])))) {
                    colon = j;
                    break;
                }
            }
            if (colon == std::string_view::npos) {
                return false;
            }

            bool ok = true;
            const auto from = yaml_scalar(trimmed.substr(0, colon), ok, false);
            if (!ok || from.empty()) {
                return false;
            }

            auto value = trim_copy(trimmed.substr(colon + 1));
            std::string to;
            if (value.starts_with('|') || value.starts_with('>')) {
                // Block scalar: gather following lines indented deeper than this entry.
                const bool folded = value.front() == '>';
                const bool strip = value.find('-') != std::string_view::npos;
                const bool keep = value.find('+') != std::string_view::npos;
                std::vector<std::string> chunk;
                std::size_t block_indent = std::string_view::npos;
                while (i + 1 < lines.size()) {
                    const auto& next = lines[i + 1];
                    if (next.text.empty()) {
                        chunk.emplace_back();
                        ++i;
                        continue;
                    }
                    if (next.indent <= indent) {
                        break;
                    }
                    if (block_indent == std::string_view::npos) {
                        block_indent = next.indent;
                    }
                    chunk.emplace_back(next.text.substr(std::min(block_indent, next.text.size())));
                    ++i;
                }
                for (std::size_t k = 0; k < chunk.size(); ++k) {
                    to += (k ? "\n" : "") + chunk[k];
                }
                if (!strip && !keep) {
                    to += '\n';
                }
                if (keep) {
                    to += '\n';
                }
                if (folded) {
                    for (char& c: to) {
                        if (c == '\n') {
                            c = ' ';
                        }
                    }
                }
            } else {
                to = yaml_scalar(value, ok, true);
                if (!ok) {
                    return false;
                }
            }
            if (to.empty() && !value.starts_with('|') && !value.starts_with('>')) {
                return false;
            }
            out.push_back(make_record(from, to, section == 301));
        }
        return saw_section;
    }

    enum class JsonParse
    {
        ok,
        malformed,
        not_array
    };

    [[nodiscard]] inline JsonParse parse_json(std::string_view payload, std::vector<Record>& out)
    {
        if (trim_copy(payload).empty()) {
            return JsonParse::malformed;
        }
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(std::string{payload});
        if (!parsed) {
            return JsonParse::malformed;
        }
        const auto arr = parsed->to_array();
        if (!arr) {
            return JsonParse::not_array; // JSON input must be an array
        }
        for (const auto& element: *arr) {
            const auto obj = element.to_object();
            if (!obj) {
                out.push_back(make_record("", "", false));
                continue;
            } // validate() -> 422
            const auto field = [&](std::string_view key) -> const serde::Value*
            {
                for (const auto& [k, v]: *obj) {
                    if (k == key) {
                        return &v;
                    }
                }
                return nullptr;
            };
            // parseJson only checks array-ness (redirect-config-parser.ts:24-31); the
            // per-entry shape check below belongs to validate() and yields 422, not 400.
            std::string from, to;
            if (const auto* p = field("from")) {
                if (const auto* s = std::get_if<std::string>(&p->get())) {
                    from = *s;
                }
            }
            if (const auto* p = field("to")) {
                if (const auto* s = std::get_if<std::string>(&p->get())) {
                    to = *s;
                }
            }
            bool permanent = false;
            if (const auto* p = field("permanent")) {
                if (const auto* b = std::get_if<bool>(&p->get())) {
                    permanent = *b;
                }
            }
            out.push_back(make_record(std::move(from), std::move(to), permanent));
        }
        return JsonParse::ok;
    }

    // JS `new RegExp(from)` validity (validation.js:43-51) approximated with a
    // structural check; std::regex is deliberately not used because its ECMAScript
    // dialect rejects some JS-valid patterns (\p{...}, named groups).
    [[nodiscard]] inline bool regex_compiles(std::string_view pattern) noexcept
    {
        int parens = 0, brackets = 0;
        bool escaped = false;
        for (std::size_t i = 0; i < pattern.size(); ++i) {
            const char c = pattern[i];
            if (escaped) {
                escaped = false;
                continue;
            }
            if (c == '\\') {
                escaped = true;
                continue;
            }
            if (c == '[') {
                ++brackets;
            } else if (c == ']') {
                if (brackets == 0) {
                    return false;
                }
                --brackets;
            } else if (c == '(') {
                ++parens;
            } else if (c == ')') {
                if (parens == 0) {
                    return false;
                }
                --parens;
            }
        }
        return !escaped && brackets == 0 && parens == 0;
    }

    // DynamicRedirectManager.buildRegex (dynamic-redirect-manager.js:30-47,106-134).
    [[nodiscard]] inline bool dry_run_loadable(std::string_view from) noexcept
    {
        std::string pattern{from};
        if (pattern.size() >= 3 && pattern.front() == '/' && pattern[pattern.size() - 2] == '/' &&
            pattern.back() == 'i') {
            pattern = pattern.substr(1, pattern.size() - 3);
        }
        if (!pattern.empty() && pattern.back() == '/') {
            pattern.pop_back();
        }
        if (pattern.empty() || pattern.back() != '$') {
            pattern += "/?$";
        }
        return regex_compiles(pattern);
    }

    [[nodiscard]] inline std::string unquote(std::string_view raw)
    {
        if (raw.size() >= 2 && raw.front() == '"') {
            if (auto s = core_serde::JsonCodec{}.decode<std::string>(std::string{raw})) {
                return *s;
            }
        }
        return std::string{raw};
    }

} // namespace upload_detail

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"redirects_redirects_upload_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const auto body = in.body_as_string();
        const auto content_type =
            std::string{in.find_header(interfaces::io::types::Token::CONTENT_TYPE)};
        const auto part = upload_detail::extract_part(body, content_type);

        // upload.validation({type:'redirects'}) (upload.js:281-303): missing file -> 422,
        // extension/content-type outside uploads.redirects -> 415.
        if (!part.found) {
            return upload_detail::fail("422", "Please select a JSON file.", "ValidationError");
        }
        const auto ext = upload_detail::extension_of(part.filename);
        if (!upload_detail::allowed_extension(ext) ||
            !upload_detail::allowed_content_type(part.mimetype)) {
            return upload_detail::fail(
                "415",
                "Please select a valid JSON file to import.",
                "UnsupportedMediaTypeError"
            );
        }

        // parseYaml/parseJson (redirect-config-parser.ts:14-96): malformed shape -> 400.
        std::vector<upload_detail::Record> parsed;
        const bool yaml = ext == ".yaml";
        if (yaml) {
            if (!upload_detail::parse_yaml(part.payload, parsed)) {
                return upload_detail::fail(
                    "400",
                    "YAML input is invalid. Check the contents of your YAML file.",
                    "BadRequestError",
                    upload_detail::kHelp
                );
            }
        } else {
            const auto jr = upload_detail::parse_json(part.payload, parsed);
            if (jr == upload_detail::JsonParse::malformed) {
                return upload_detail::fail(
                    "400",
                    "Could not parse JSON: invalid JSON input.",
                    "BadRequestError",
                    upload_detail::kHelp
                );
            }
            if (jr == upload_detail::JsonParse::not_array) {
                return upload_detail::fail(
                    "400",
                    "JSON input must be an array of redirect objects.",
                    "BadRequestError",
                    upload_detail::kHelp
                );
            }
        }

        // RedirectsService.replace (redirects-service.ts:79-104): validate() and the
        // dry-run _verifyAllLoadable() both report 422 ValidationError.
        for (const auto& rec: parsed) {
            const auto from = upload_detail::unquote(rec.at("from"));
            const auto to = upload_detail::unquote(rec.at("to"));
            if (upload_detail::trim_copy(from).empty() || upload_detail::trim_copy(to).empty()) {
                return upload_detail::fail(
                    "422",
                    "Incorrect redirects file format.",
                    "ValidationError",
                    upload_detail::kHelp
                );
            }
            if (!upload_detail::regex_compiles(from)) {
                return upload_detail::fail(
                    "422",
                    "Incorrect RegEx in redirects file.",
                    "ValidationError",
                    upload_detail::kHelp
                );
            }
            if (!upload_detail::dry_run_loadable(from)) {
                return upload_detail::fail(
                    "422",
                    std::format("Could not register redirect: {} -> {}", from, to),
                    "ValidationError",
                    upload_detail::kHelp
                );
            }
        }

        // replace-all (not create-one): remove every existing redirects record first.
        auto& rs = ctx.store().resources();
        for (const auto& rec: blog::api::parse_array(rs.list("redirects"))) {
            const auto it = rec.find("id");
            if (it == rec.end()) {
                continue;
            }
            const auto id =
                core_serde::JsonCodec{}.decode<std::string>(it->second).value_or(it->second);
            rs.destroy("redirects", id);
        }
        for (const auto& rec: parsed) {
            rs.create("redirects", blog::api::record_to_json(rec));
        }

        // Ghost api.replace() returns void -> res.json(result || {}) -> {}.
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", "{}"}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::redirects::redirects_upload_post::workflow
