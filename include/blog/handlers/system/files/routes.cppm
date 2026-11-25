export module blog_handlers_routes:files_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

namespace blog::api {

namespace {

    /** @brief Ghost `config.uploads.files.extensions` (core/shared/config/overrides.json:70-176).
     */
    constexpr std::array<std::string_view, 104> kFilesExtensions{
        ".7z",    ".aac",     ".ai",      ".aif",
        ".aiff",  ".apkg",    ".arw",     ".avif",
        ".avi",   ".blend",   ".bmp",     ".bz2",
        ".cr2",   ".cr3",     ".css",     ".csv",
        ".deb",   ".dng",     ".doc",     ".docx",
        ".eot",   ".epub",    ".eps",     ".fb2",
        ".fbx",   ".flac",    ".geojson", ".gif",
        ".glb",   ".gltf",    ".gpx",     ".gz",
        ".heic",  ".heif",    ".html",    ".ics",
        ".ipynb", ".ipuz",    ".jpeg",    ".jpg",
        ".js",    ".json",    ".jsonld",  ".key",
        ".kml",   ".m4a",     ".m4v",     ".md",
        ".mid",   ".midi",    ".mkv",     ".mobi",
        ".mov",   ".mp3",     ".mp4",     ".nef",
        ".np3",   ".numbers", ".obj",     ".odp",
        ".ods",   ".odt",     ".ogv",     ".opus",
        ".orf",   ".otf",     ".pages",   ".paprikarecipes",
        ".pdf",   ".png",     ".ppt",     ".pptx",
        ".psd",   ".py",      ".raf",     ".rar",
        ".rpm",   ".rtf",     ".rw2",     ".skp",
        ".stl",   ".svg",     ".tar",     ".tgz",
        ".tif",   ".tiff",    ".toml",    ".tsv",
        ".ttf",   ".txt",     ".vcf",     ".wav",
        ".webm",  ".webp",    ".woff",    ".woff2",
        ".xcf",   ".xls",     ".xlsb",    ".xlsm",
        ".xlsx",  ".xml",     ".yaml",    ".zip",
    };

    std::string lower(std::string_view text)
    {
        std::string out;
        out.reserve(text.size());
        for (const char c: text) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
        return out;
    }

    std::string ext_of(std::string_view filename)
    {
        const auto dot = filename.find_last_of('.');
        return dot == std::string_view::npos ? std::string{} : lower(filename.substr(dot));
    }

    /** @brief Reads the `filename="..."` of the multipart part named `file` (multer +
     * fileValidation). */
    std::string upload_filename(std::string_view content_type, std::string_view body)
    {
        if (!content_type.starts_with("multipart/form-data")) {
            return {};
        }
        constexpr std::string_view name_marker{"name=\"file\""};
        constexpr std::string_view filename_marker{"filename=\""};
        auto pos = body.find(name_marker);
        while (pos != std::string_view::npos) {
            auto block_end = body.find("\r\n\r\n", pos);
            if (block_end == std::string_view::npos) {
                block_end = body.size();
            }
            const auto fn = body.find(filename_marker, pos);
            if (fn != std::string_view::npos && fn < block_end) {
                const auto begin = fn + filename_marker.size();
                const auto end = body.find('"', begin);
                if (end != std::string_view::npos && end < block_end && end > begin) {
                    return std::string{body.substr(begin, end - begin)};
                }
            }
            pos = body.find(name_marker, pos + name_marker.size());
        }
        return {};
    }

    using blog::handlers::middleware::ErrorEnvelope;
    using blog::handlers::middleware::ErrorItem;
    using blog::handlers::middleware::write_error;

    [[nodiscard]] bool
    write_missing_file(interfaces::io::IResponse& response, std::function<void()> send)
    {
        write_error(
            response,
            interfaces::io::types::Status::BAD_REQUEST,
            ErrorEnvelope{std::vector{
                ErrorItem{.message = "Please select a file.", .type = "ValidationError"}
            }},
            std::move(send)
        );
        return false;
    }

    [[nodiscard]] bool
    write_unsupported_file(interfaces::io::IResponse& response, std::function<void()> send)
    {
        write_error(
            response,
            interfaces::io::types::Status::UNSUPPORTED_MEDIA_TYPE,
            ErrorEnvelope{std::vector{ErrorItem{
                .message = "The file type you uploaded is not supported. You can zip your file and "
                           "upload it as a .zip.",
                .type = "UnsupportedMediaTypeError"
            }}},
            std::move(send)
        );
        return false;
    }

    /**
     * @brief `apiMw.upload.single('file')` + `fileValidation({type:'files'})` stand-in.
     * Ghost emits a 400 ValidationError for a missing file and a 415
     * UnsupportedMediaTypeError for an extension outside config.uploads.files.extensions
     * (web/api/middleware/upload.js:442-458).
     */
    void require_upload_file(
        interfaces::io::IRequest& request,
        interfaces::io::IResponse& response,
        interfaces::NextFn&& next,
        std::function<void()> send
    )
    {
        const auto filename = upload_filename(request.get_content_type(), request.body_as_string());
        if (filename.empty()) {
            (void)write_missing_file(response, std::move(send));
            return;
        }
        const auto ext = ext_of(filename);
        if (std::ranges::find(kFilesExtensions, std::string_view{ext}) == kFilesExtensions.end()) {
            (void)write_unsupported_file(response, std::move(send));
            return;
        }
        next(request, response, std::move(send));
    }

} // namespace

export class FilesRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Ghost has no JSON-only content-type gate on this route: multer consumes the
        // multipart body directly. The shared `content_type_middleware` would 415 a
        // genuine upload, so the chain intentionally omits it (recorded central gap).
        Router{router, "/files"}
            .use(blog::handlers::middleware::require_auth)
            .use(require_upload_file)
            .add_route(
                Route{"/upload"}
                    .post(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.files_files_upload_post().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost upload POST")
            );
    }
};

} // namespace blog::api
