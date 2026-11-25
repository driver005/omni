module;
#include <ctime>
export module blog_handlers:images_images_upload_post_step;
import :domain_all;
import :outbox_shim;
import std;
import core_workflow;
import interfaces;

/**
 * Strict-parity stand-in for Ghost `POST /images/upload` (`images.js:11-91`).
 *
 * Ghost's controller has no model call: it resolves the `storage:images` adapter,
 * normalizes the filename, optionally resizes via sharp, then calls `store.save()`
 * once (no-resize) or twice (processed + `_o` original). The registered module set
 * has no multipart-to-disk lifecycle, sharp, or storage adapter, so the fetch/save
 * chain is represented on the shared outbox (`blog_handlers:outbox_shim`,
 * CONTRACTS.md §4) with a locally registered adapter. Real image bytes, SVG
 * sanitization, and the adapter itself are recorded as new-partition needs in
 * docs/parity/behavior/images.md — no shared file or BUILD edit is made here.
 */
export namespace blog::handlers::images::images_upload_post::workflow {

namespace support {

    inline constexpr std::string_view kResizeEvent{"image.transform.resize"};
    inline constexpr std::string_view kSvgSanitizeEvent{"image.svg.sanitize"};
    inline constexpr std::string_view kSaveEvent{"image.store.save"};

    inline constexpr std::string_view kMissingFile{"Please select an image."};
    inline constexpr std::string_view kInvalidFile{"Please select a valid image."};
    inline constexpr std::string_view kResizeFailure{"Image processing failed"};

    inline constexpr int kDefaultMaxWidth{
        2'000
    }; // imageOptimization:defaultMaxWidth (overrides.json:252)

    [[nodiscard]] inline std::string lower(std::string_view text)
    {
        std::string out;
        out.reserve(text.size());
        for (const char c: text) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
        return out;
    }

    [[nodiscard]] inline std::string json_quote(std::string_view text)
    {
        std::string out{"\""};
        for (const char c: text) {
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
                    out.push_back(c);
            }
        }
        out.push_back('"');
        return out;
    }

    [[nodiscard]] inline std::string json_field(std::string_view json, std::string_view key)
    {
        const std::string needle = std::format("\"{}\"", key);
        const auto at = json.find(needle);
        if (at == std::string_view::npos) {
            return {};
        }
        const auto colon = json.find(':', at + needle.size());
        if (colon == std::string_view::npos) {
            return {};
        }
        auto rest = json.substr(colon + 1);
        while (!rest.empty() && std::isspace(static_cast<unsigned char>(rest.front()))) {
            rest.remove_prefix(1);
        }
        if (rest.empty() || rest.front() != '"') {
            return {};
        }
        rest.remove_prefix(1);
        std::string out;
        for (std::size_t i = 0; i < rest.size(); ++i) {
            if (rest[i] == '\\' && i + 1 < rest.size()) {
                ++i;
                switch (rest[i]) {
                    case 'n':
                        out.push_back('\n');
                        break;
                    case 'r':
                        out.push_back('\r');
                        break;
                    case 't':
                        out.push_back('\t');
                        break;
                    default:
                        out.push_back(rest[i]);
                }
            } else if (rest[i] == '"') {
                break;
            } else {
                out.push_back(rest[i]);
            }
        }
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
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
            value.remove_prefix(1);
        }
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
            value.remove_suffix(1);
        }
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        }
        return std::string{value};
    }

    [[nodiscard]] inline std::string quoted_param(std::string_view headers, std::string_view key)
    {
        const std::string needle = std::format("{}=\"", key);
        const auto pos = headers.find(needle);
        if (pos == std::string_view::npos) {
            return {};
        }
        const auto begin = pos + needle.size();
        const auto end = headers.find('"', begin);
        if (end == std::string_view::npos) {
            return {};
        }
        return std::string{headers.substr(begin, end - begin)};
    }

    /** @brief Multipart part `Content-Type: image/png` is unquoted, unlike `name="file"`. */
    [[nodiscard]] inline std::string part_content_type(std::string_view headers)
    {
        constexpr std::string_view key{"content-type:"};
        auto pos = headers.find(key);
        if (pos == std::string_view::npos) {
            return {};
        }
        pos += key.size();
        while (pos < headers.size() && std::isspace(static_cast<unsigned char>(headers[pos]))) {
            ++pos;
        }
        const auto end = headers.find("\r\n", pos);
        auto value =
            headers.substr(pos, end == std::string_view::npos ? std::string_view::npos : end - pos);
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
            value.remove_suffix(1);
        }
        return std::string{value};
    }

    struct UploadedFile
    {
        std::string filename{};
        std::string mime{};
        std::string bytes{};
    };

    /** @brief Multipart part iterator: returns the headers and byte bounds of each part. */
    template<class Fn>
    inline void for_each_part(std::string_view body, std::string_view boundary, Fn&& fn)
    {
        if (boundary.empty()) {
            return;
        }
        const std::string delimiter = "--" + std::string{boundary};
        std::size_t cursor = 0;
        while (cursor < body.size()) {
            const auto start = body.find(delimiter, cursor);
            if (start == std::string_view::npos) {
                break;
            }
            cursor = start + delimiter.size();
            if (body.substr(cursor, 2) == "--") {
                break;
            }
            while (cursor < body.size() && (body[cursor] == '\r' || body[cursor] == '\n')) {
                ++cursor;
            }

            const auto header_end = body.find("\r\n\r\n", cursor);
            if (header_end == std::string_view::npos) {
                break;
            }
            const auto headers = body.substr(cursor, header_end - cursor);
            const auto part_begin = header_end + 4;

            auto next = body.find(delimiter, part_begin);
            if (next == std::string_view::npos) {
                next = body.size();
            }
            auto part_end = next;
            while (part_end > part_begin &&
                   (body[part_end - 1] == '\n' || body[part_end - 1] == '\r')) {
                --part_end;
            }
            fn(headers, body.substr(part_begin, part_end - part_begin));
            cursor = next;
        }
    }

    /** @brief `upload.single('file')`: the part named `file` with a filename. */
    [[nodiscard]] inline UploadedFile parse_upload(std::string_view body, std::string_view boundary)
    {
        UploadedFile file{};
        for_each_part(
            body,
            boundary,
            [&](std::string_view headers, std::string_view bytes)
            {
                if (!file.filename.empty() || quoted_param(headers, "name") != "file") {
                    return;
                }
                const auto filename = quoted_param(headers, "filename");
                if (filename.empty()) {
                    return;
                }
                file.filename = filename;
                file.mime = part_content_type(headers);
                file.bytes = std::string{bytes};
            }
        );
        return file;
    }

    /** @brief `frame.data.ref`: the optional multipart form field, echoed by the serializer. */
    [[nodiscard]] inline std::string
    form_field(std::string_view body, std::string_view boundary, std::string_view name)
    {
        std::string value{};
        for_each_part(
            body,
            boundary,
            [&](std::string_view headers, std::string_view bytes)
            {
                if (value.empty() && quoted_param(headers, "name") == name) {
                    value = std::string{bytes};
                }
            }
        );
        return value;
    }

    /** @brief `path.extname(name).toLowerCase()` (upload.js:294). */
    [[nodiscard]] inline std::string ext_of(std::string_view filename)
    {
        const auto slash = filename.find_last_of('/');
        const auto base = slash == std::string_view::npos ? filename : filename.substr(slash + 1);
        const auto dot = base.find_last_of('.');
        if (dot == std::string_view::npos || dot == 0) {
            return {};
        }
        return lower(base.substr(dot));
    }

    /** @brief `name.replace(/_o(\.\w+?)$/, '$1')` (images.js:25). */
    [[nodiscard]] inline std::string normalize_o_suffix(std::string name, std::string_view ext)
    {
        if (ext.empty() || name.size() <= ext.size()) {
            return name;
        }
        const auto stem_end = name.size() - ext.size();
        if (stem_end >= 2 && name.compare(stem_end - 2, 2, "_o") == 0) {
            name.erase(stem_end - 2, 2);
        }
        return name;
    }

    [[nodiscard]] inline bool allowed_ext(std::string_view ext)
    {
        static const std::set<std::string_view>
            allowed{".jpg", ".jpeg", ".gif", ".png", ".svg", ".svgz", ".ico", ".webp"};
        return allowed.contains(ext);
    }

    [[nodiscard]] inline bool allowed_type(std::string_view type)
    {
        static const std::set<std::string_view> allowed{
            "image/jpeg",
            "image/png",
            "image/gif",
            "image/svg+xml",
            "image/x-icon",
            "image/vnd.microsoft.icon",
            "image/webp"
        };
        return allowed.contains(type);
    }

    [[nodiscard]] inline bool is_svg(std::string_view ext)
    {
        return ext == ".svg" || ext == ".svgz";
    }

    /** @brief `shouldResizeFileExtension` (image-transform/src/index.ts:103-104). */
    [[nodiscard]] inline bool should_resize(std::string_view ext)
    {
        return !is_svg(ext) && ext != ".ico";
    }

    /** @brief `generateOriginalImageName` (image-transform/src/index.ts:223-226). */
    [[nodiscard]] inline std::string original_name(std::string_view name, std::string_view ext)
    {
        if (ext.empty()) {
            return std::string{name} + "_o";
        }
        return std::string{name.substr(0, name.size() - ext.size())} + "_o" + std::string{ext};
    }

    /** @brief StorageBase.getSanitizedFileName: replace everything outside [\w@.] with '-'. */
    [[nodiscard]] inline std::string sanitize_filename(std::string_view name)
    {
        std::string out;
        out.reserve(name.size());
        for (const unsigned char c: name) {
            const bool keep = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                              (c >= '0' && c <= '9') || c == '_' || c == '@' || c == '.';
            out.push_back(keep ? static_cast<char>(c) : '-');
        }
        return out;
    }

    /** @brief StorageBase.getTargetDir: YYYY/MM from the server clock (moment()). */
    [[nodiscard]] inline std::string target_dir()
    {
        const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm parts{};
#if defined(_WIN32)
        localtime_s(&parts, &now);
#else
        localtime_r(&now, &parts);
#endif
        char buf[16]{};
        std::strftime(buf, sizeof(buf), "%Y/%m", &parts);
        return std::string{buf};
    }

    /** @brief StorageBase.getUniqueFileName + generateUnique stand-in (in-process). */
    [[nodiscard]] inline std::string unique_filename(std::string_view filename)
    {
        const auto dot = filename.find_last_of('.');
        const std::string ext =
            dot == std::string_view::npos ? std::string{} : std::string{filename.substr(dot)};
        const std::string stem =
            sanitize_filename(dot == std::string_view::npos ? filename : filename.substr(0, dot));

        static std::set<std::string> issued{};
        static std::mutex issued_mutex{};
        const std::lock_guard<std::mutex> lock{issued_mutex};

        auto candidate = stem + ext;
        if (issued.insert(candidate).second) {
            return candidate;
        }
        for (int i = 1;; ++i) {
            candidate = stem + "-" + std::to_string(i) + ext;
            if (issued.insert(candidate).second) {
                return candidate;
            }
        }
    }

    [[nodiscard]] inline std::string env_or(std::string_view name, std::string_view fallback)
    {
        const char* value = std::getenv(std::string{name}.c_str());
        return value == nullptr ? std::string{fallback} : std::string{value};
    }

    [[nodiscard]] inline std::string site_url(const interfaces::io::IRequest& request)
    {
        const auto override = request.find_header("x-blog-site-url");
        std::string value =
            override.empty() ? env_or("SITE_URL", "http://localhost:2368") : std::string{override};
        while (!value.empty() && value.back() == '/') {
            value.pop_back();
        }
        return value;
    }

    /** @brief Ghost `urlUtils.urlFor('image', {image:path}, true)` — absolutize the saved path. */
    [[nodiscard]] inline std::string absolute_url(std::string_view relative, std::string_view site)
    {
        std::string path{relative};
        if (!path.starts_with('/')) {
            path.insert(path.begin(), '/');
        }
        return std::string{site} + path;
    }

    struct StoreResult
    {
        bool attempted{false};
        bool resize_failed{false};
        std::string resize_error{};
        std::string processed_url{};      // relative path returned by the processed/no-resize save
        std::vector<std::string> saves{}; // every `store.save` recorded in order
    };

    [[nodiscard]] inline StoreResult& last_store() noexcept
    {
        static StoreResult result;
        return result;
    }

    /**
     * @brief Outbox adapter standing in for `imageTransform.resizeFromPath` and
     *        `adapterManager.getAdapter('storage:images').save(...)`. It assigns the
     *        Ghost `content/images/YYYY/MM/<sanitized-unique-name>` path for each save
     *        and records the save on the outbox (drain point). No bytes are written.
     */
    class ImageStoreAdapter final : public blog::handlers::outbox::Adapter
    {
    public:
        void dispatch(const blog::handlers::outbox::Event& event) override
        {
            if (event.type == kResizeEvent) {
                auto& result = last_store();
                result.attempted = true;
                if (json_field(event.payload, "fail") == "true") {
                    result.resize_failed = true;
                    result.resize_error = "Input buffer contains unsupported image format";
                }
            } else if (event.type == kSvgSanitizeEvent) {
                // No sanitizer partition is registered; accepted stand-in (new-partition need).
            } else if (event.type == kSaveEvent) {
                const bool original = json_field(event.payload, "original") == "true";
                const std::string dir = json_field(event.payload, "dir");
                const std::string subdir = json_field(event.payload, "subdir");
                const std::string name = json_field(event.payload, "name");
                const std::string url = "/" + subdir + (subdir.empty() ? "" : "/") +
                                        "content/images/" + dir + "/" + unique_filename(name);
                auto& result = last_store();
                result.saves.push_back(url);
                if (!original) {
                    result.processed_url = url;
                }
            }
        }
    };

    inline void ensure_adapter()
    {
        static const bool registered = []
        {
            blog::handlers::outbox::register_adapter(std::make_shared<ImageStoreAdapter>());
            return true;
        }();
        (void)registered;
    }

    [[nodiscard]] inline const StoreResult&
    run_transform(std::string_view ext, std::string_view tmp_path, bool force_failure)
    {
        ensure_adapter();
        last_store() = StoreResult{};
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        outbox.emit(
            blog::handlers::outbox::Event{
                std::string{kResizeEvent},
                std::format(
                    R"({{"in":{},"ext":{},"width":{},"fail":{}}})",
                    json_quote(tmp_path),
                    json_quote(ext),
                    kDefaultMaxWidth,
                    force_failure ? "true" : "false"
                ),
                std::string{tmp_path}
            }
        );
        outbox.commit();
        [[maybe_unused]] const auto drained = outbox.drain();
        return last_store();
    }

    [[nodiscard]] inline const StoreResult& run_saves(
        std::string_view name,
        std::string_view original,
        std::string_view ext,
        std::string_view type,
        std::string_view dir,
        std::string_view subdir,
        bool resized
    )
    {
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        const auto emit_save = [&](std::string_view save_name, bool is_original)
        {
            outbox.emit(
                blog::handlers::outbox::Event{
                    std::string{kSaveEvent},
                    std::format(
                        R"({{"name":{},"ext":{},"type":{},"dir":{},"subdir":{},"original":{}}})",
                        json_quote(save_name),
                        json_quote(ext),
                        json_quote(type),
                        json_quote(dir),
                        json_quote(subdir),
                        is_original ? "true" : "false"
                    ),
                    std::string{save_name}
                }
            );
        };
        if (resized) {
            emit_save(name, false);    // processed image is saved first (images.js:57-60)
            emit_save(original, true); // then the `_o` original (images.js:77-84)
        } else {
            emit_save(name, false); // store.save(frame.file) (images.js:89)
        }
        outbox.commit();
        [[maybe_unused]] const auto drained = outbox.drain();
        return last_store();
    }

    [[nodiscard]] inline core::workflow::StepResponse<std::map<std::string, std::string>>
    fail(int status, std::string body)
    {
        return core::workflow::StepResponse<std::map<std::string, std::string>>{
            std::map<std::string, std::string>{
                {"status", std::to_string(status)},
                {"body", std::move(body)}
            }
        };
    }

} // namespace support

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"images_images_upload_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        // Retained for the handler-ctor wiring contract. Ghost's controller makes no
        // model call, so the old `store().create("images", ...)` marker is retired;
        // the storage side effect is observable through the outbox instead.
        (void)s_ctx;

        const std::string body = in.body_as_string();
        const std::string boundary = support::boundary_of(in.get_content_type());
        const auto file = support::parse_upload(body, boundary);
        if (file.filename.empty() || file.bytes.empty()) {
            return support::fail(
                422,
                blog::api::ApiError{}.validation(support::kMissingFile, "", "")
            );
        }

        const std::string ext = support::ext_of(file.filename);
        const std::string type = support::lower(file.mime.empty() ? std::string{} : file.mime);
        if (!support::allowed_ext(ext) || !support::allowed_type(type)) {
            return support::fail(
                415,
                blog::api::ApiError{}
                    .validation(support::kInvalidFile, "", "", "UnsupportedMediaTypeError")
            );
        }

        // images.js:25 — trim a client-side `_o` suffix before any save.
        const std::string name = support::normalize_o_suffix(file.filename, ext);
        const std::string subdir{in.find_header("x-blog-subdir")};
        const std::string dir = support::target_dir();

        const bool resized = support::should_resize(ext); // imageOptimization.resize defaults true
        const bool force_failure = in.find_header("x-blog-transform-fail") == "true";

        if (support::is_svg(ext)) {
            // Sanitizer has no registered partition; the event is the stand-in.
            support::ensure_adapter();
            auto& outbox = blog::handlers::outbox::global_outbox();
            outbox.emit(
                blog::handlers::outbox::Event{
                    std::string{support::kSvgSanitizeEvent},
                    std::format(R"({{"name":{}}})", support::json_quote(name)),
                    name
                }
            );
            [[maybe_unused]] const auto drained = outbox.drain();
        }

        bool transform_ok = true;
        if (resized) {
            const std::string tmp_path = "/tmp/" + name;
            const auto& result = support::run_transform(ext, tmp_path, force_failure);
            if (result.resize_failed) {
                const std::string detail = result.resize_error;
                const std::string error = std::format(
                    R"({{"errors":[{{"id":"{:032x}","code":null,"context":{},"details":null,"ghostErrorCode":null,"help":{},"message":{},"property":null,"type":"BadRequestError"}}]}})",
                    std::uint64_t{1},
                    support::json_quote(detail),
                    support::json_quote("Please verify that the image is valid"),
                    support::json_quote(support::kResizeFailure)
                );
                return support::fail(400, error);
            }
            transform_ok = result.attempted;
        }
        (void)transform_ok;

        const std::string original = support::original_name(name, ext);
        const auto& result = support::run_saves(name, original, ext, type, dir, subdir, resized);
        const std::string url = support::absolute_url(result.processed_url, support::site_url(in));
        const std::string ref = support::form_field(body, boundary, "ref");

        // Output serializer `images.upload`: {images:[{url: forImage(path), ref: data.ref||null}]}
        const std::string response = std::format(
            R"({{"images":[{{"url":{},"ref":{}}}]}})",
            support::json_quote(url),
            ref.empty() ? std::string{"null"} : support::json_quote(ref)
        );
        return support::fail(201, response);
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::images::images_upload_post::workflow
