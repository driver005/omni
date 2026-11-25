export module blog_handlers:media_media_upload_post_step;
import :domain_all;
import :outbox_shim;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::media::media_upload_post::workflow {

namespace detail {

    /** @brief Ghost's multipart part plus the form fields (`ref`, ...) that travel alongside it. */
    struct Part
    {
        std::string name{};
        std::string filename{};
        std::string type{};
        std::string data{};

        [[nodiscard]] bool is_file() const noexcept
        {
            return !filename.empty();
        }
    };

    struct Multipart
    {
        std::vector<Part> parts{};
        bool well_formed{false};
    };

    [[nodiscard]] inline std::string lower(std::string_view value)
    {
        std::string out{value};
        for (char& c: out) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return out;
    }

    [[nodiscard]] inline std::size_t find_ci(std::string_view haystack, std::string_view needle)
    {
        if (needle.empty()) {
            return 0;
        }
        if (needle.size() > haystack.size()) {
            return std::string_view::npos;
        }
        for (std::size_t i = 0; i + needle.size() <= haystack.size(); ++i) {
            bool match = true;
            for (std::size_t j = 0; j < needle.size(); ++j) {
                if (std::tolower(static_cast<unsigned char>(haystack[i + j])) !=
                    std::tolower(static_cast<unsigned char>(needle[j]))) {
                    match = false;
                    break;
                }
            }
            if (match) {
                return i;
            }
        }
        return std::string_view::npos;
    }

    [[nodiscard]] inline std::string json_quote(std::string_view value)
    {
        std::string out{"\""};
        for (const char c: value) {
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
                    if (static_cast<unsigned char>(c) < 0x20) {
                        out += std::format(
                            "\\u{:04x}",
                            static_cast<unsigned>(static_cast<unsigned char>(c))
                        );
                    } else {
                        out += c;
                    }
            }
        }
        out += '"';
        return out;
    }

    [[nodiscard]] inline std::string boundary_of(std::string_view content_type)
    {
        constexpr std::string_view marker{"boundary="};
        const auto pos = content_type.find(marker);
        if (pos == std::string_view::npos) {
            return {};
        }
        auto value = content_type.substr(pos + marker.size());
        if (const auto semi = value.find(';'); semi != std::string_view::npos) {
            value = value.substr(0, semi);
        }
        while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
            value.remove_prefix(1);
        }
        while (!value.empty() && (value.back() == ' ' || value.back() == '\t' ||
                                  value.back() == '\r' || value.back() == '\n')) {
            value.remove_suffix(1);
        }
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        }
        return std::string{value};
    }

    /** @brief Reads `name="..."` / `filename="..."` / `Content-Type: ...` from a part header. */
    [[nodiscard]] inline std::string header_value(std::string_view headers, std::string_view key)
    {
        std::size_t from = 0;
        for (;;) {
            const auto pos = headers.find(key, from);
            if (pos == std::string_view::npos) {
                return {};
            }
            if (pos > 0 && std::isalnum(static_cast<unsigned char>(headers[pos - 1]))) {
                from = pos + key.size();
                continue;
            }
            auto value = headers.substr(pos + key.size());
            while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
                value.remove_prefix(1);
            }
            if (!value.empty() && value.front() == '"') {
                const auto end = value.find('"', 1);
                return end == std::string_view::npos ? std::string{}
                                                     : std::string{value.substr(1, end - 1)};
            }
            const auto end = value.find_first_of(";\r\n");
            std::string result{value.substr(0, end)};
            while (!result.empty() && (result.back() == ' ' || result.back() == '\t')) {
                result.pop_back();
            }
            return result;
        }
    }

    /** @brief Mirrors Ghost `getStorageContentType` (`lib/file-types.ts:38-52`). */
    [[nodiscard]] inline std::string storage_content_type(std::string_view filename)
    {
        const auto dot = filename.find_last_of('.');
        const std::string ext =
            dot == std::string_view::npos ? std::string{} : lower(filename.substr(dot));

        if (ext == ".html" || ext == ".htm" || ext == ".js" || ext == ".css" || ext == ".xml") {
            return "text/plain";
        }
        if (ext == ".jpg" || ext == ".jpeg") {
            return "image/jpeg";
        }
        if (ext == ".png") {
            return "image/png";
        }
        if (ext == ".gif") {
            return "image/gif";
        }
        if (ext == ".webp") {
            return "image/webp";
        }
        if (ext == ".svg" || ext == ".svgz") {
            return "image/svg+xml";
        }
        if (ext == ".mp4") {
            return "video/mp4";
        }
        if (ext == ".webm") {
            return "video/webm";
        }
        if (ext == ".ogv") {
            return "video/ogg";
        }
        if (ext == ".mp3") {
            return "audio/mpeg";
        }
        if (ext == ".wav") {
            return "audio/wave";
        }
        if (ext == ".ogg") {
            return "audio/ogg";
        }
        if (ext == ".m4a") {
            return "audio/mp4";
        }
        return "application/octet-stream";
    }

    /**
     * @brief Splits a `multipart/form-data` body. `well_formed` is false for a body without a
     * boundary or without the closing `--boundary--`, matching Ghost's multer 400 on a broken body
     * (`e2e-api/admin/media.test.js`: "Errors when media request body is broken").
     */
    [[nodiscard]] inline Multipart
    parse_multipart(std::string_view content_type, std::string_view body)
    {
        Multipart out;
        const std::string boundary = boundary_of(content_type);
        if (boundary.empty()) {
            return out;
        }
        const std::string delimiter = "--" + boundary;
        if (body.find(delimiter + "--") == std::string_view::npos) {
            return out;
        }
        out.well_formed = true;

        std::size_t cursor = 0;
        while (cursor <= body.size()) {
            const auto start = body.find(delimiter, cursor);
            if (start == std::string_view::npos) {
                break;
            }
            std::size_t segment_start = start + delimiter.size();
            if (body.substr(segment_start, 2) == "--") {
                break;
            }
            if (body.substr(segment_start, 2) == "\r\n") {
                segment_start += 2;
            } else if (segment_start < body.size() && body[segment_start] == '\n') {
                segment_start += 1;
            }

            const auto next = body.find(delimiter, segment_start);
            const std::size_t segment_end = next == std::string_view::npos ? body.size() : next;
            auto segment = body.substr(segment_start, segment_end - segment_start);
            while (!segment.empty() && (segment.back() == '\r' || segment.back() == '\n')) {
                segment.remove_suffix(1);
            }

            std::string_view headers;
            std::string_view data;
            const auto sep = segment.find("\r\n\r\n");
            if (sep != std::string_view::npos) {
                headers = segment.substr(0, sep);
                data = segment.substr(sep + 4);
            } else if (const auto lf = segment.find("\n\n"); lf != std::string_view::npos) {
                headers = segment.substr(0, lf);
                data = segment.substr(lf + 2);
            } else {
                cursor = segment_end;
                continue;
            }

            Part part;
            part.name = header_value(headers, "name=");
            part.filename = header_value(headers, "filename=");
            part.data = std::string{data};
            const std::string declared = header_value(headers, "Content-Type:");
            part.type = declared.empty() ? storage_content_type(part.filename) : lower(declared);
            if (!part.name.empty()) {
                out.parts.push_back(std::move(part));
            }
            cursor = segment_end;
        }
        return out;
    }

    [[nodiscard]] inline const Part*
    find_part(const std::vector<Part>& parts, std::string_view name)
    {
        for (const Part& part: parts) {
            if (part.name == name && part.is_file()) {
                return &part;
            }
        }
        return nullptr;
    }

    [[nodiscard]] inline std::string
    field_value(const std::vector<Part>& parts, std::string_view name)
    {
        for (const Part& part: parts) {
            if (part.name == name && !part.is_file()) {
                return part.data;
            }
        }
        return {};
    }

    [[nodiscard]] inline std::string extension(std::string_view filename)
    {
        const auto slash = filename.find_last_of("/\\");
        const auto name = slash == std::string_view::npos ? filename : filename.substr(slash + 1);
        const auto dot = name.find_last_of('.');
        return dot == std::string_view::npos ? std::string{} : std::string{name.substr(dot)};
    }

    [[nodiscard]] inline std::string stem(std::string_view filename)
    {
        const auto slash = filename.find_last_of("/\\");
        const auto name = slash == std::string_view::npos ? filename : filename.substr(slash + 1);
        const auto dot = name.find_last_of('.');
        return dot == std::string_view::npos ? std::string{name} : std::string{name.substr(0, dot)};
    }

    /** @brief Ghost `save()` keeps the original name when free, else generates a unique one. */
    [[nodiscard]] inline std::string
    unique_name(std::string_view filename, std::set<std::string>& used)
    {
        const std::string ext = extension(filename);
        const std::string base = stem(filename);
        std::string candidate{filename};
        for (std::size_t n = 1; used.contains(candidate); ++n) {
            candidate = base + "-" + std::to_string(n) + ext;
        }
        used.insert(candidate);
        return candidate;
    }

    /** @brief `config.uploads.media` (`shared/config/overrides.json:53-69`). */
    [[nodiscard]] inline bool is_media(const std::string& type, std::string_view ext)
    {
        static constexpr std::string_view kTypes[]{
            "video/mp4",
            "video/webm",
            "video/ogg",
            "audio/mpeg",
            "audio/vnd.wav",
            "audio/wave",
            "audio/wav",
            "audio/x-wav",
            "audio/ogg",
            "application/ogg",
            "audio/mp4",
            "audio/x-m4a"
        };
        static constexpr std::string_view
            kExts[]{".mp4", ".webm", ".ogv", ".mp3", ".wav", ".ogg", ".m4a"};
        return std::ranges::find(kTypes, std::string_view{type}) != std::ranges::end(kTypes) &&
               std::ranges::find(kExts, ext) != std::ranges::end(kExts);
    }

    /** @brief `config.uploads.thumbnails` (`shared/config/overrides.json:179-190`). */
    [[nodiscard]] inline bool is_thumbnail_type(const std::string& type, std::string_view ext)
    {
        static constexpr std::string_view kTypes[]{
            "image/jpeg",
            "image/png",
            "image/gif",
            "image/svg+xml",
            "image/x-icon",
            "image/vnd.microsoft.icon",
            "image/webp"
        };
        static constexpr std::string_view
            kExts[]{".jpg", ".jpeg", ".gif", ".png", ".svg", ".svgz", ".ico", ".webp"};
        return std::ranges::find(kTypes, std::string_view{type}) != std::ranges::end(kTypes) &&
               std::ranges::find(kExts, ext) != std::ranges::end(kExts);
    }

    [[nodiscard]] inline bool is_svg(std::string_view ext, const std::string& type)
    {
        return ext == ".svg" || ext == ".svgz" || type == "image/svg+xml";
    }

    /**
     * @brief Std-only stand-in for Ghost `sanitizeSvgFile` (`lib/image/svg-sanitizer`). Rejects
     * malformed SVGs (no `<svg>` root), drops `<script>` blocks and inline event handlers, and
     * neutralises `javascript:` URLs. The real sanitizer is an XML parser and is recorded as a
     * residual gap; this covers the observable e2e cases (`media.test.js`: "Sanitizes SVG
     * thumbnails", "Rejects invalid SVG thumbnails").
     */
    [[nodiscard]] inline bool sanitize_svg(std::string& svg)
    {
        if (find_ci(svg, "<svg") == std::string_view::npos) {
            return false;
        }
        for (;;) {
            const auto open = find_ci(svg, "<script");
            if (open == std::string_view::npos) {
                break;
            }
            const auto close = find_ci(std::string_view{svg}.substr(open), "</script");
            if (close == std::string_view::npos) {
                svg.erase(open);
                break;
            }
            const auto end = svg.find('>', open + close);
            svg.erase(open, (end == std::string::npos ? svg.size() : end + 1) - open);
        }
        for (std::size_t i = 0; i + 1 < svg.size();) {
            const bool boundary = std::isspace(static_cast<unsigned char>(svg[i])) != 0;
            const bool on = boundary && find_ci(std::string_view{svg}.substr(i + 1, 2), "on") == 0;
            if (!on) {
                ++i;
                continue;
            }
            std::size_t j = i + 3;
            while (j < svg.size() && std::isalnum(static_cast<unsigned char>(svg[j]))) {
                ++j;
            }
            if (j == i + 3) {
                ++i;
                continue;
            }
            while (j < svg.size() && std::isspace(static_cast<unsigned char>(svg[j]))) {
                ++j;
            }
            if (j >= svg.size() || svg[j] != '=') {
                i = j;
                continue;
            }
            ++j;
            while (j < svg.size() && std::isspace(static_cast<unsigned char>(svg[j]))) {
                ++j;
            }
            if (j >= svg.size() || (svg[j] != '"' && svg[j] != '\'')) {
                i = j;
                continue;
            }
            const char quote = svg[j++];
            const auto end = svg.find(quote, j);
            svg.erase(i, (end == std::string::npos ? svg.size() : end + 1) - i);
        }
        for (;;) {
            const auto js = find_ci(svg, "javascript:");
            if (js == std::string_view::npos) {
                break;
            }
            svg.replace(js, 11, "blocked:");
        }
        return true;
    }

    /** @brief Std-only stand-in for `isAllowedImageContent` (`lib/image/image-content`). */
    [[nodiscard]] inline bool allowed_image_content(std::string_view data, std::string_view ext)
    {
        auto starts = [data](std::string_view magic)
        {
            return data.size() >= magic.size() && data.substr(0, magic.size()) == magic;
        };
        if (ext == ".jpg" || ext == ".jpeg") {
            return starts("\xFF\xD8\xFF");
        }
        if (ext == ".png") {
            return starts("\x89PNG\r\n\x1A\n");
        }
        if (ext == ".gif") {
            return starts("GIF87a") || starts("GIF89a");
        }
        if (ext == ".webp") {
            return data.size() >= 12 && starts("RIFF") && data.substr(8, 4) == "WEBP";
        }
        if (ext == ".ico") {
            return starts("\x00\x00\x01\x00");
        }
        return true; // unknown extension: nothing to sniff
    }

    /** @brief `getURL` (`utils/serializers/output/media.js:4-16`) with site URL from the request.
     */
    [[nodiscard]] inline std::string
    get_url(const interfaces::io::IRequest& in, std::string_view path)
    {
        constexpr std::string_view kPrefix{"/content/media/"};
        if (!path.starts_with(kPrefix)) {
            return std::string{path}; // CDN / absolute URL: unchanged
        }
        std::string base{in.get_scheme()};
        base += "://";
        base += in.get_authority();
        while (!base.empty() && base.back() == '/') {
            base.pop_back();
        }
        if (base == "://") {
            return std::string{path}; // no request origin available
        }
        return base + std::string{path};
    }

    [[nodiscard]] inline std::pair<std::string, std::string> current_year_month()
    {
        const auto now = std::chrono::system_clock::now();
        const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};
        const auto year = static_cast<int>(ymd.year());
        const auto month = static_cast<unsigned>(ymd.month());
        return {std::format("{:04d}", year), std::format("{:02d}", month)};
    }

    [[nodiscard]] inline std::string
    meta(std::string_view name, std::string_view path, std::string_view type)
    {
        return "{\"name\":" + json_quote(name) + ",\"path\":" + json_quote(path) +
               ",\"type\":" + json_quote(type) + "}";
    }

    /** @brief Emits the observable stand-in for `storage.save()` (`adapter-manager`/local fs). */
    inline void record_save(
        blog::BlogContext& ctx,
        std::string_view name,
        std::string_view path,
        std::string_view type
    )
    {
        (void)ctx.store().create("media", meta(name, path, type));
        blog::handlers::outbox::global_outbox().emit(
            blog::handlers::outbox::Event{
                .type = "media.saved",
                .payload = meta(name, path, type),
                .aggregate_id = std::string{path}
            }
        );
    }

} // namespace detail

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"media_media_upload_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        using namespace detail;

        const auto multipart = parse_multipart(in.get_content_type(), in.body_as_string());
        if (!multipart.well_formed) {
            return problem("400", "The request could not be understood.", "", "BadRequestError");
        }

        const Part* file = find_part(multipart.parts, "file");
        if (file == nullptr) {
            return problem("422", "Please select a media file.", "file", "ValidationError");
        }
        const std::string fileExt = extension(file->filename);
        if (!is_media(file->type, fileExt)) {
            return problem(
                "415",
                "Please select a valid media file.",
                "file",
                "UnsupportedMediaTypeError"
            );
        }

        const Part* thumbnail = find_part(multipart.parts, "thumbnail");
        std::string thumbnailPath;
        if (thumbnail != nullptr) {
            const std::string thumbExt = extension(thumbnail->filename);
            if (!is_thumbnail_type(thumbnail->type, thumbExt)) {
                return problem(
                    "415",
                    "Please select a valid thumbnail.",
                    "thumbnail",
                    "UnsupportedMediaTypeError"
                );
            }
            std::string bytes = thumbnail->data;
            if (is_svg(thumbExt, thumbnail->type)) {
                if (!sanitize_svg(bytes)) {
                    return problem(
                        "415",
                        "Please select a valid SVG image",
                        "thumbnail",
                        "UnsupportedMediaTypeError"
                    );
                }
            } else if (!allowed_image_content(bytes, thumbExt)) {
                return problem(
                    "415",
                    "Please select a valid thumbnail.",
                    "thumbnail",
                    "UnsupportedMediaTypeError"
                );
            }
            const std::string thumbName = stem(file->filename) + "_thumb" + thumbExt;
            const auto [year, month] = current_year_month();
            std::set<std::string> used;
            thumbnailPath =
                "/content/media/" + year + "/" + month + "/" + unique_name(thumbName, used);
            record_save(ctx, thumbName, thumbnailPath, thumbnail->type);
        }

        const auto [year, month] = current_year_month();
        std::set<std::string> fileUsed;
        const std::string filePath =
            "/content/media/" + year + "/" + month + "/" + unique_name(file->filename, fileUsed);
        record_save(ctx, file->filename, filePath, file->type);

        const std::string ref = field_value(multipart.parts, "ref");
        const std::string body =
            "{\"media\":[{\"url\":" + json_quote(get_url(in, filePath)) + ",\"thumbnail_url\":" +
            (thumbnailPath.empty() ? std::string{"null"} : json_quote(get_url(in, thumbnailPath))) +
            ",\"ref\":" + (ref.empty() ? std::string{"null"} : json_quote(ref)) + "}]}";
        return core::workflow::StepResponse<Output>{{{"status", "201"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static core::workflow::StepResponse<Output> problem(
        std::string_view status,
        std::string_view message,
        std::string_view property,
        std::string_view type
    )
    {
        return core::workflow::StepResponse<Output>{
            {{"status", std::string{status}},
             {"body", blog::api::ApiError{}.validation(message, "", property, type)}}
        };
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::media::media_upload_post::workflow
