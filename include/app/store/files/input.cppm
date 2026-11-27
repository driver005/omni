export module app_store:files_input;

import std;
import interfaces;
import core_error;
import core_behavior;
import app_context;
import app_context_core;
import app_http;

export namespace app::handlers::files {

/**
 * @brief Local mirror of upstream `types::api::files::FilePurpose`
 * (`hyperswitch_interfaces/src/api/files.rs:14-31`, `types/api/files.rs:3`).
 * V1 has exactly one variant.
 */
enum class FilePurpose : std::uint8_t
{
    DisputeEvidence
};

/**
 * @brief Local mirror of upstream `types::api::files::CreateFileRequest`
 * (`types/api/files.rs:60-70`). Constructed by `get_create_file_request` from the raw
 * multipart body, exactly like the upstream transformer (`routes/files/transformers.rs:13-82`).
 */
struct CreateFileRequestData
{
    std::string file{};
    std::optional<std::string> file_name{};
    std::int32_t file_size{};
    std::string file_type{};
    FilePurpose purpose{FilePurpose::DisputeEvidence};
    std::optional<std::string> dispute_id{};
};

/** @brief One parsed multipart field (name, optional filename, raw content bytes). */
struct MultipartPart
{
    std::string name{};
    std::optional<std::string> filename{};
    std::string content{};
};

/**
 * @brief Parse/auth/path support for the files handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime. The multipart body is decoded here
 * because the local HTTP layer exposes bytes, not Actix `Multipart` fields.
 */
class FilesInputSupport
{
public:
    explicit FilesInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    /**
     * @brief Upstream files auth: HeaderAuth(ApiKeyAuth{allow_connected_scope_operation=true,
     * allow_platform_self_operation=false}) or DashboardNoPermissionAuth{allow_connected=true,
     * allow_platform=false} (`files.rs:52-62,104-114,158-168`).
     */
    [[nodiscard]] std::optional<core::error::AppError>
    authorize_request(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto base = m_context.auth_rules().rule_for(operation);
        const auto rule = ::app::auth::RouteAuthRuleBuilder{}
                              .scheme(::app::auth::AuthScheme::API_KEY)
                              .permission(base.permission())
                              .allow_connected(true)
                              .allow_platform(false)
                              .build();
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    /** @brief Upstream `platform.get_processor().get_account().get_id()` substitute. */
    [[nodiscard]] std::string
    scope_merchant_id(interfaces::io::IRequest& req, const ::app::auth::AuthContext& auth) const
    {
        const auto header = ::app::http::merchant_id_from_header{}(req);
        return header.empty() ? auth.merchant_id() : header;
    }

    /**
     * @brief Port of `get_create_file_request` (`routes/files/transformers.rs:13-82`).
     */
    [[nodiscard]] std::expected<CreateFileRequestData, core::error::AppError>
    get_create_file_request(interfaces::io::IRequest& req, const std::string& body) const
    {
        const std::string content_type = ::app::http::HeaderReader{}.get(req, "content-type");
        auto parts = parse_multipart(body, content_type);
        if (!parts) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Invalid multipart request"
            )};
        }

        std::optional<FilePurpose> purpose{};
        std::optional<std::string> dispute_id{};
        std::optional<std::string> file_name{};
        std::optional<std::string> file_content{};

        // transformers.rs:22-51 — iterate fields; last occurrence wins, like upstream.
        for (const auto& part: *parts) {
            if (part.name == "purpose") {
                purpose = get_file_purpose(part);
            } else if (part.name == "file") {
                file_name = part.filename;
                file_content = part.content;
            } else if (part.name == "dispute_id") {
                dispute_id = read_string(part);
            }
        }

        // transformers.rs:52 — option_purpose.get_required_value("purpose").
        if (!purpose.has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "File purpose not found in the request or is invalid"
            )};
        }
        // transformers.rs:53-57 — MissingFile when no file part.
        if (!file_content.has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "File not found / valid in the request"
            )};
        }
        // transformers.rs:59-61 — i32::try_from(file.len()).
        if (file_content->size() >
            static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max())) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "File size error"
            )};
        }
        const auto file_size = static_cast<std::int32_t>(file_content->size());
        // transformers.rs:62-66 — empty file is MissingFile.
        if (file_size <= 0) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "File not found / valid in the request"
            )};
        }
        // transformers.rs:67-73 — infer::get(&file) + mime parse.
        const auto mime = infer_mime(*file_content);
        if (!mime.has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "File content type not found / valid"
            )};
        }

        CreateFileRequestData result{};
        result.file = std::move(*file_content);
        result.file_name = std::move(file_name);
        result.file_size = file_size;
        result.file_type = std::string{*mime};
        result.purpose = *purpose;
        result.dispute_id = std::move(dispute_id);
        return result;
    }

private:
    /** @brief `helpers::read_string` (`core/files/helpers.rs:17-24`). */
    [[nodiscard]] std::optional<std::string> read_string(const MultipartPart& part) const
    {
        return part.content;
    }

    /** @brief `helpers::get_file_purpose` (`core/files/helpers.rs:26-32`). */
    [[nodiscard]] std::optional<FilePurpose> get_file_purpose(const MultipartPart& part) const
    {
        const auto value = read_string(part);
        if (value.has_value() &&
            std::string_view{*value} == file_purpose_name(FilePurpose::DisputeEvidence)) {
            return FilePurpose::DisputeEvidence;
        }
        return std::nullopt;
    }

    /** @brief Local parser for `multipart/form-data` bodies (Actix `Multipart` substitute). */
    [[nodiscard]] std::optional<std::vector<MultipartPart>>
    parse_multipart(const std::string& body, const std::string& content_type) const
    {
        const auto boundary = multipart_boundary(content_type);
        if (!boundary.has_value() || boundary->empty()) {
            return std::nullopt;
        }
        const std::string delimiter{"--" + *boundary};
        std::vector<MultipartPart> parts{};
        std::size_t cursor{0};
        while (cursor < body.size()) {
            const std::size_t found = body.find(delimiter, cursor);
            if (found == std::string::npos) {
                break;
            }
            std::size_t start = found + delimiter.size();
            if (body.compare(start, 2, "--") == 0) {
                break; // closing boundary
            }
            if (body.compare(start, 2, "\r\n") == 0) {
                start += 2;
            } else if (start < body.size() && body[start] == '\n') {
                start += 1;
            }
            const std::size_t next = body.find(delimiter, start);
            std::size_t end = (next == std::string::npos) ? body.size() : next;
            if (end >= start + 2 && body.compare(end - 2, 2, "\r\n") == 0) {
                end -= 2;
            } else if (end > start && body[end - 1] == '\n') {
                end -= 1;
            }
            const std::string section = body.substr(start, end - start);
            const std::size_t header_end = section.find("\r\n\r\n");
            if (header_end != std::string::npos) {
                MultipartPart part{};
                part.content = section.substr(header_end + 4);
                const std::string headers = section.substr(0, header_end);
                std::size_t line_start{0};
                while (line_start <= headers.size()) {
                    const std::size_t line_end = headers.find("\r\n", line_start);
                    const std::size_t length =
                        (line_end == std::string::npos ? headers.size() : line_end) - line_start;
                    const std::string line = headers.substr(line_start, length);
                    if (to_lower(line).starts_with("content-disposition:")) {
                        part.name = multipart_attribute(line, "name").value_or(std::string{});
                        part.filename = multipart_attribute(line, "filename");
                    }
                    if (line_end == std::string::npos) {
                        break;
                    }
                    line_start = line_end + 2;
                }
                if (!part.name.empty()) {
                    parts.push_back(std::move(part));
                }
            }
            cursor = end;
        }
        return parts;
    }

    [[nodiscard]] std::optional<std::string> multipart_boundary(std::string_view content_type) const
    {
        const std::string lower = to_lower(content_type);
        if (!lower.starts_with("multipart/form-data")) {
            return std::nullopt;
        }
        const std::size_t pos = lower.find("boundary=");
        if (pos == std::string::npos) {
            return std::nullopt;
        }
        std::size_t start = pos + 9;
        while (start < content_type.size() &&
               std::isspace(static_cast<unsigned char>(content_type[start]))) {
            ++start;
        }
        if (start >= content_type.size()) {
            return std::nullopt;
        }
        if (content_type[start] == '"') {
            const std::size_t end = content_type.find('"', start + 1);
            if (end == std::string::npos) {
                return std::nullopt;
            }
            return std::string{content_type.substr(start + 1, end - start - 1)};
        }
        std::size_t end = start;
        while (end < content_type.size() && content_type[end] != ';' &&
               !std::isspace(static_cast<unsigned char>(content_type[end]))) {
            ++end;
        }
        return std::string{content_type.substr(start, end - start)};
    }

    [[nodiscard]] std::optional<std::string>
    multipart_attribute(std::string_view line, std::string_view key) const
    {
        const std::size_t pos = line.find(key);
        if (pos == std::string::npos) {
            return std::nullopt;
        }
        const std::size_t eq = line.find('=', pos + key.size());
        if (eq == std::string::npos) {
            return std::nullopt;
        }
        std::size_t start = eq + 1;
        while (start < line.size() && std::isspace(static_cast<unsigned char>(line[start]))) {
            ++start;
        }
        if (start >= line.size()) {
            return std::nullopt;
        }
        if (line[start] == '"') {
            const std::size_t end = line.find('"', start + 1);
            if (end == std::string::npos) {
                return std::nullopt;
            }
            return std::string{line.substr(start + 1, end - start - 1)};
        }
        std::size_t end = start;
        while (end < line.size() && line[end] != ';' &&
               !std::isspace(static_cast<unsigned char>(line[end]))) {
            ++end;
        }
        return std::string{line.substr(start, end - start)};
    }

    /**
     * @brief Local port of the `infer` crate's magic-byte detection used at
     * `routes/files/transformers.rs:68`. Returns the mime the upstream `Kind::mime_type()`
     * reports for the recognized signature (or nullopt -> MissingFileContentType).
     */
    [[nodiscard]] std::optional<std::string_view> infer_mime(std::string_view bytes) const
    {
        const auto starts = [bytes](std::string_view signature)
        {
            return bytes.size() >= signature.size() &&
                   bytes.substr(0, signature.size()) == signature;
        };
        if (starts("%PDF-")) {
            return "application/pdf";
        }
        if (bytes.size() >= 8 && static_cast<unsigned char>(bytes[0]) == 0x89 && bytes[1] == 'P' &&
            bytes[2] == 'N' && bytes[3] == 'G' && bytes[4] == 0x0D && bytes[5] == 0x0A &&
            bytes[6] == 0x1A && bytes[7] == 0x0A) {
            return "image/png";
        }
        if (bytes.size() >= 3 && static_cast<unsigned char>(bytes[0]) == 0xFF &&
            static_cast<unsigned char>(bytes[1]) == 0xD8 &&
            static_cast<unsigned char>(bytes[2]) == 0xFF) {
            return "image/jpeg";
        }
        if (starts("GIF87a") || starts("GIF89a")) {
            return "image/gif";
        }
        if (bytes.size() >= 12 && starts("RIFF") && bytes.substr(8, 4) == "WEBP") {
            return "image/webp";
        }
        if (bytes.size() >= 12 && starts("RIFF") && bytes.substr(8, 4) == "WAVE") {
            return "audio/wav";
        }
        if (starts("PK\x03\x04") || starts("PK\x05\x06") || starts("PK\x07\x08")) {
            return "application/zip";
        }
        if (bytes.size() >= 8 && bytes.substr(4, 4) == "ftyp") {
            return "video/mp4";
        }
        if (starts("BM")) {
            return "image/bmp";
        }
        if (bytes.size() >= 5 && bytes.substr(0, 5) == "<?xml") {
            return "text/xml";
        }
        const std::size_t first = bytes.find_first_not_of(" \t\r\n");
        if (first != std::string_view::npos && (bytes[first] == '{' || bytes[first] == '[')) {
            return "application/json";
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string_view file_purpose_name(FilePurpose purpose) const noexcept
    {
        (void)purpose; // V1 has a single variant: dispute_evidence.
        return "dispute_evidence";
    }

    [[nodiscard]] std::string to_lower(std::string_view value) const
    {
        std::string out{};
        out.reserve(value.size());
        for (const char character: value) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
        }
        return out;
    }

    ::app::context::AppContext& m_context;
};

/**
 * @brief Domain policy consumed by core::behavior::RequestSupport.
 *
 * Files auth (API key with connected-scope allowed) and the processor merchant
 * scope are the domain rules; the file id is a generic path segment and the
 * dispute id is a generic query parameter, both owned by RequestSupport.
 */
class FilesPolicy
{
public:
    explicit FilesPolicy(FilesInputSupport& support) noexcept :
        m_support{support}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        (void)spec; // the files rule is registry-derived and fixed by operation.
        return m_support.authorize_request(req, operation);
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        core::behavior::Scope result;
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        result.merchant_id = m_support.scope_merchant_id(req, auth);
        return result;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope&) const noexcept
    {
        return std::nullopt;
    }

private:
    FilesInputSupport& m_support;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old *_core call received.

struct FilesCreateInput
{
    CreateFileRequestData request{};
    std::string processor_merchant_id{};
    std::string provider_merchant_id{};
};

struct FilesDeleteInput
{
    std::string processor_merchant_id{};
    std::string file_id{};
};

struct FilesRetrieveInput
{
    std::string processor_merchant_id{};
    std::string file_id{};
    std::optional<std::string> dispute_id{};
};

} // namespace app::handlers::files
