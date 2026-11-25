module;
#include <ctime>
export module blog_handlers:files_files_upload_post_step;
import :domain_all;
import :files_files_upload_post_dto;
import std;
import core_workflow;
import core_serde;
import interfaces;

namespace blog::handlers::files::files_upload_post::workflow {

namespace {

    std::string lower(std::string_view text)
    {
        std::string out;
        out.reserve(text.size());
        for (const char c: text) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
        return out;
    }

    /** @brief Pulls a quoted `key="value"` parameter out of a multipart header block. */
    std::string quoted_param(std::string_view headers, std::string_view key)
    {
        const std::string needle = std::string{key} + "=\"";
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

    /** @brief Extracts the boundary parameter of `multipart/form-data`. */
    std::string boundary_of(std::string_view content_type)
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

    struct UploadedFile
    {
        std::string filename{};
        std::string bytes{};
    };

    /** @brief Minimal multipart parser: finds the part with `name="file"`. */
    UploadedFile parse_upload(std::string_view body, std::string_view boundary)
    {
        UploadedFile result{};
        if (boundary.empty()) {
            return result;
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
                break; // closing delimiter
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

            if (quoted_param(headers, "name") == "file") {
                result.filename = quoted_param(headers, "filename");
                result.bytes = std::string{body.substr(part_begin, part_end - part_begin)};
                return result;
            }
            cursor = next;
        }
        return result;
    }

    /** @brief Reads `frame.data.ref` — the multipart form field `ref` (Ghost output serializer). */
    std::string form_field(std::string_view body, std::string_view boundary, std::string_view name)
    {
        if (boundary.empty()) {
            return {};
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

            if (quoted_param(headers, "name") == name) {
                return std::string{body.substr(part_begin, part_end - part_begin)};
            }
            cursor = next;
        }
        return {};
    }

    /** @brief Mirrors Ghost `getStorageContentType` (`lib/file-types.ts:4-52`). */
    std::string storage_content_type(std::string_view filename)
    {
        const auto dot = filename.find_last_of('.');
        const std::string ext =
            dot == std::string_view::npos ? std::string{} : lower(filename.substr(dot));

        // Tier 1: CONTENT_TYPE_OVERRIDES — forced to text/plain to stop browser execution.
        if (ext == ".html" || ext == ".htm" || ext == ".js" || ext == ".css" || ext == ".xml") {
            return "text/plain";
        }

        // Tier 2: mime-types.lookup(ext) filtered by BROWSER_RENDERABLE_TYPES.
        // The allowlist also includes text/plain (mime-types maps .txt here).
        if (ext == ".txt") {
            return "text/plain";
        }
        if (ext == ".jpg" || ext == ".jpeg" || ext == ".jpe") {
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
        if (ext == ".avif") {
            return "image/avif";
        }
        if (ext == ".bmp") {
            return "image/bmp";
        }
        if (ext == ".pdf") {
            return "application/pdf";
        }
        if (ext == ".json") {
            return "application/json";
        }
        if (ext == ".mp3") {
            return "audio/mpeg";
        }
        if (ext == ".wav") {
            return "audio/wave";
        }
        if (ext == ".m4a") {
            return "audio/mp4";
        }
        if (ext == ".mp4") {
            return "video/mp4";
        }
        if (ext == ".mov") {
            return "video/quicktime";
        }
        if (ext == ".webm") {
            return "video/webm";
        }
        if (ext == ".ogv") {
            return "video/ogg";
        }
        if (ext == ".otf") {
            return "font/otf";
        }
        if (ext == ".ttf") {
            return "font/ttf";
        }
        if (ext == ".woff") {
            return "font/woff";
        }
        if (ext == ".woff2") {
            return "font/woff2";
        }

        // Tier 3: fallback forces a download.
        return "application/octet-stream";
    }

    /** @brief StorageBase.getSanitizedFileName: replace everything outside [\w@.] with '-'. */
    std::string sanitize_filename(std::string_view name)
    {
        std::string out;
        out.reserve(name.size());
        for (const unsigned char c: name) {
            const bool allowed = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                                 (c >= '0' && c <= '9') || c == '_' || c == '@' || c == '.';
            out.push_back(allowed ? static_cast<char>(c) : '-');
        }
        return out;
    }

    /** @brief StorageBase.getTargetDir: YYYY/MM from the server clock. */
    std::string target_dir()
    {
        const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        const std::tm* parts = std::localtime(&now);
        char buf[16]{};
        if (parts == nullptr) {
            return "1970/01";
        }
        std::strftime(buf, sizeof(buf), "%Y/%m", parts);
        return std::string{buf};
    }

    /**
     * @brief StorageBase.getUniqueFileName + generateUnique stand-in.
     * Ghost probes the filesystem for collisions; with no storage adapter in the
     * registered modules this tracks already-issued names in-process (recorded gap).
     */
    std::string unique_filename(std::string_view filename)
    {
        const auto dot = filename.find_last_of('.');
        const bool numeric_ext = dot != std::string_view::npos &&
                                 std::all_of(
                                     filename.begin() + static_cast<std::ptrdiff_t>(dot) + 1,
                                     filename.end(),
                                     [](const char c)
                                     {
                                         return c >= '0' && c <= '9';
                                     }
                                 );
        const std::string ext = (dot == std::string_view::npos || numeric_ext)
                                    ? std::string{}
                                    : std::string{filename.substr(dot)};
        const auto stem_source =
            (dot == std::string_view::npos || numeric_ext) ? filename : filename.substr(0, dot);
        const std::string stem = sanitize_filename(stem_source);

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

    std::string env_or(std::string_view name, std::string_view fallback)
    {
        const char* value = std::getenv(std::string{name}.c_str());
        return value == nullptr ? std::string{fallback} : std::string{value};
    }

    /** @brief Harness-configurable site URL (Ghost config.getSiteUrl()). */
    std::string site_url(const interfaces::io::IRequest& request)
    {
        const auto override = request.find_header("x-blog-site-url");
        std::string value =
            override.empty() ? env_or("SITE_URL", "http://localhost:2368") : std::string{override};
        while (!value.empty() && value.back() == '/') {
            value.pop_back();
        }
        return value;
    }

    /** @brief Ghost output serializer `getURL`: absolutize a /content/files/... path. */
    std::string
    absolute_url(std::string_view relative, std::string_view site, std::string_view subdir)
    {
        std::string path{relative};
        if (!path.starts_with('/')) {
            path.insert(path.begin(), '/');
        }

        const std::string prefix = std::string{subdir} + "/content/files/";
        if (!path.starts_with(prefix)) {
            return path; // Ghost leaves non-matching paths untouched.
        }

        path.erase(0, subdir.size());
        return std::string{site} + path;
    }

} // namespace

export class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"files_files_upload_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        const auto boundary = boundary_of(in.get_content_type());
        const auto body = in.body_as_string();
        const UploadedFile file = parse_upload(body, boundary);
        const std::string type = storage_content_type(file.filename);
        const std::string ref = form_field(body, boundary, "ref");

        const auto subdir = std::string{in.find_header("x-blog-subdir")};
        // LocalFilesStorage.save -> LocalStorageBase.save: /<subdir>/content/files/YYYY/MM/<name>.
        const std::string storedPath =
            subdir + "/content/files/" + target_dir() + "/" + unique_filename(file.filename);
        const std::string url = absolute_url(storedPath, site_url(in), subdir);

        // No storage-adapter abstraction in the registered modules: the generic `files`
        // resource holds the storage-save metadata {name, path, type} as a stand-in.
        blog::dto::files::FileMetaDto meta;
        meta.set_name(file.filename);
        meta.set_path(storedPath);
        meta.set_type(type);
        (void)ctx.store().create("files", core_serde::JsonCodec{}.encode(meta));

        return core::workflow::StepResponse<Output>{
            Output{{"url", url}, {"ref", ref}, {"filePath", storedPath}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::files::files_upload_post::workflow
