module;
export module app_handlers:files;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_error;
import app_auth;
import app_http;
import app_metrics;
import app_store;
import app_connector;
import app_services;

export namespace app::handlers::files {

/** @brief Upstream `ApplicationResponse::StatusOk` wire shape. */
class StatusOkWire
{
public:
    void set_status(std::string value) { status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return status; }
    std::string status{};
};

/**
 * @brief Local serializable shape for upstream multipart `CreateFileRequest`.
 * @note The local HTTP layer carries JSON rather than multipart. `file` holds the text
 * bytes of the uploaded part; `file_name`, `file_size` and `file_type` are optional and
 * are derived from it when absent, mirroring `get_create_file_request` + `infer`.
 */
class CreateFileRequestWire
{
public:
    void set_file(std::optional<std::string> value) { file = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_file() const noexcept { return file; }
    void set_file_name(std::optional<std::string> value) { file_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_file_name() const noexcept { return file_name; }
    void set_file_size(std::optional<std::int64_t> value) { file_size = std::move(value); }
    [[nodiscard]] const std::optional<std::int64_t>& get_file_size() const noexcept { return file_size; }
    void set_file_type(std::optional<std::string> value) { file_type = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_file_type() const noexcept { return file_type; }
    void set_purpose(std::optional<std::string> value) { purpose = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_purpose() const noexcept { return purpose; }
    void set_dispute_id(std::optional<std::string> value) { dispute_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_dispute_id() const noexcept { return dispute_id; }

    std::optional<std::string> file{};
    std::optional<std::string> file_name{};
    std::optional<std::int64_t> file_size{};
    std::optional<std::string> file_type{};
    std::optional<std::string> purpose{};
    std::optional<std::string> dispute_id{};
};

} // namespace app::handlers::files

template<>
struct serde::Serializable<app::handlers::files::StatusOkWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"status", &app::handlers::files::StatusOkWire::get_status, &app::handlers::files::StatusOkWire::set_status, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "status_ok_wire"; }
};

template<>
struct serde::Serializable<app::handlers::files::CreateFileRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"file", &app::handlers::files::CreateFileRequestWire::get_file, &app::handlers::files::CreateFileRequestWire::set_file, serde::FieldOptions{}>{},
            serde::FieldDesc<"file_name", &app::handlers::files::CreateFileRequestWire::get_file_name, &app::handlers::files::CreateFileRequestWire::set_file_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"file_size", &app::handlers::files::CreateFileRequestWire::get_file_size, &app::handlers::files::CreateFileRequestWire::set_file_size, serde::FieldOptions{}>{},
            serde::FieldDesc<"file_type", &app::handlers::files::CreateFileRequestWire::get_file_type, &app::handlers::files::CreateFileRequestWire::set_file_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"purpose", &app::handlers::files::CreateFileRequestWire::get_purpose, &app::handlers::files::CreateFileRequestWire::set_purpose, serde::FieldOptions{}>{},
            serde::FieldDesc<"dispute_id", &app::handlers::files::CreateFileRequestWire::get_dispute_id, &app::handlers::files::CreateFileRequestWire::set_dispute_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "create_file_request_wire"; }
};

export namespace app::handlers::files {

class FilesRouteHandlers
{
    using Error = core::error::AppError;
    using Kind = core::error::AppErrorKind;
    using Builder = core::error::AppErrorBuilder;
    using FileRecord = app::store::files::FileRecord;

    /** @brief Local mirror of upstream `types::api::files::FileDataRequired`. */
    enum class FileDataRequired : std::uint8_t { Required, NotRequired };

public:
    explicit FilesRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    void handle_files_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("files_create");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, Builder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::deserialize<CreateFileRequestWire>("application/json", body);
        if (!parsed) {
            send_error(res, send, Builder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Multipart substitute: derive size, name and mime the way get_create_file_request
        // + infer do for an uploaded part.
        const std::string file_bytes = parsed->get_file().value_or(std::string{});
        if (file_bytes.empty()) {
            send_error(res, send, Builder{}(Kind::VALIDATION_FAILED, "file content is empty"));
            return;
        }
        std::string file_name = parsed->get_file_name().value_or(std::string{});
        if (file_name.empty()) {
            file_name = "file";
        }
        std::int64_t file_size = parsed->get_file_size().value_or(static_cast<std::int64_t>(file_bytes.size()));
        if (file_size <= 0) {
            file_size = static_cast<std::int64_t>(file_bytes.size());
        }
        const std::string file_type = infer_mime_type(file_name, parsed->get_file_type().value_or(std::string{}));
        const std::string purpose = parsed->get_purpose().value_or(std::string{});
        const std::string dispute_id = parsed->get_dispute_id().value_or(std::string{});

        const auto merchant_id = scope_merchant_id(req, auth_context);
        const auto profile_id = scope_profile_id(req, auth_context);

        // validate_file_upload substitute: FilePurpose::DisputeEvidence requires a dispute id
        // and a dispute lookup before connector validation.
        if (std::string_view{purpose} == purpose_dispute_evidence()) {
            if (dispute_id.empty()) {
                send_error(res, send, Builder{}(Kind::INVALID_REQUEST, "missing dispute id"));
                return;
            }
            const auto dispute = m_context->repository().find_dispute_by_string(dispute_id);
            if (!dispute) {
                send_error(res, send, Builder{}(Kind::NOT_FOUND, "dispute not found"));
                return;
            }
            if (const auto invalid = validate_file_upload(merchant_id, dispute_id, file_type, file_size)) {
                send_error(res, send, *invalid);
                return;
            }
        }

        m_context->metrics().record_flow("files.create");
        m_context->metrics().counter("files_create_total").increment();

        // generate_id(ID_LENGTH, "file") substitute: file_ + 20 suffix chars.
        const auto file_id = generate_file_id();
        if (find_file_for_scope(merchant_id, file_id)) {
            send_error(res, send, Builder{}(Kind::ALREADY_EXISTS, "file already exists"));
            return;
        }

        FileRecord record{};
        record.set_file_id(file_id);
        record.set_merchant_id(merchant_id);
        record.set_profile_id(profile_id);
        record.set_file_name(file_name);
        record.set_file_type(file_type);
        record.set_file_size(file_size);
        record.set_purpose(purpose);
        record.set_status(std::string{file_status_unavailable()});
        record.set_created_at(now_string());

        // insert_file_metadata: available = false until the upload completes.
        auto created = m_context->repository().create_file(record);
        if (!created) {
            send_error(res, send, created.error());
            return;
        }
        record = *created;

        // upload_and_get_provider_provider_file_id_profile_id substitute. Bytes go to the
        // local object-store substitute; the selected connector supplies the provider id.
        std::string provider_file_id = file_id;
        const std::string file_key = std::format("{}/{}", merchant_id, file_id);
        app::connector::ConnectorRequest upload_request{};
        upload_request.set_operation("submit_evidence");
        upload_request.set_merchant_id(merchant_id);
        upload_request.set_resource_id(file_key);
        upload_request.set_raw_payload(file_bytes);
        auto connector = m_context->connector_router().select_for(upload_request);
        if (!connector) {
            connector = m_context->connector_router().select(merchant_id);
        }
        if (connector) {
            const auto response = connector->submit_evidence(upload_request);
            if (!response.get_reference_id().empty()) {
                provider_file_id = response.get_reference_id();
            }
            m_context->process_trackers().schedule(
                std::string{"file_upload_"} + file_id,
                std::format("{{\"file_id\":\"{}\",\"provider\":\"{}\",\"provider_file_id\":\"{}\"}}",
                            file_id, connector->name(), provider_file_id),
                "0"
            );
            m_context->metrics().counter("files_upload_connector_total").increment();
        } else {
            m_context->metrics().counter("files_upload_local_total").increment();
        }
        m_context->config_store().put(file_storage_namespace(), file_id, file_bytes);

        // update_file_metadata: available = false -> true with provider id and upload provider.
        record.set_provider_file_id(provider_file_id);
        record.set_upload_provider(std::string{file_upload_provider_router()});
        record.set_status(std::string{file_status_available()});
        m_context->repository().update_file(record);

        app::dto::files::CreateFileResponse response{};
        response.set_file_id(file_id);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_files_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("files_delete");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto file_id = file_id_from_path(req);
        if (!file_id.has_value() || file_id->empty()) {
            send_error(res, send, Builder{}(Kind::INVALID_REQUEST, "missing file id in path"));
            return;
        }
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto merchant_id = scope_merchant_id(req, auth_context);

        m_context->metrics().record_flow("files.delete");
        m_context->metrics().counter("files_delete_total").increment();

        // delete_file_metadata_by_processor_merchant_id_file_id with processor scoping.
        const auto record = find_file_for_scope(merchant_id, *file_id);
        if (!record) {
            send_error(res, send, Builder{}(Kind::NOT_FOUND, "file not found"));
            return;
        }
        if (std::string_view{record->get_status()} != file_status_available() || record->get_provider_file_id().empty()) {
            send_error(res, send, Builder{}(Kind::PRECONDITION_FAILED, "file not available"));
            return;
        }
        if (std::string_view{record->get_upload_provider()} != file_upload_provider_router()) {
            send_error(res, send, Builder{}(Kind::VALIDATION_FAILED, "file provider not supported"));
            return;
        }

        // delete_file_using_file_id substitute: remove bytes from the local object store.
        m_context->config_store().remove(file_storage_namespace(), *file_id);
        m_context->process_trackers().schedule(
            std::string{"file_delete_"} + *file_id,
            std::format("{{\"file_id\":\"{}\",\"provider_file_id\":\"{}\"}}",
                        *file_id, record->get_provider_file_id()),
            "0"
        );
        m_context->repository().remove_file(record->get_id());

        StatusOkWire response{};
        response.set_status("success");
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_files_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("files_retrieve");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto file_id = file_id_from_path(req);
        if (!file_id.has_value() || file_id->empty()) {
            send_error(res, send, Builder{}(Kind::INVALID_REQUEST, "missing file id in path"));
            return;
        }
        const auto query = app::http::RequestQuery{}(req);
        const auto dispute_id = query.get("dispute_id");
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto merchant_id = scope_merchant_id(req, auth_context);

        m_context->metrics().record_flow("files.retrieve");
        m_context->metrics().counter("files_retrieve_total").increment();

        // find_file_metadata_by_processor_merchant_id_file_id -> FileNotFound when absent.
        const auto record = find_file_for_scope(merchant_id, *file_id);
        if (!record) {
            send_error(res, send, Builder{}(Kind::NOT_FOUND, "file not found"));
            return;
        }
        if (std::string_view{record->get_status()} != file_status_available()) {
            send_error(res, send, Builder{}(Kind::PRECONDITION_FAILED, "file not available"));
            return;
        }
        // file_type.parse::<mime::Mime>() guard.
        if (!is_valid_mime(record->get_file_type())) {
            send_error(res, send, Builder{}(Kind::INTERNAL_ERROR, "failed to parse file content type"));
            return;
        }

        // retrieve_file_and_provider_file_id_from_file_id + FileDataRequired::Required.
        const FileDataRequired selector = FileDataRequired::Required;
        const auto bytes = m_context->config_store().get_value(file_storage_namespace(), *file_id);
        if (!bytes && selector == FileDataRequired::Required) {
            send_error(res, send, Builder{}(Kind::PRECONDITION_FAILED, "file not available"));
            return;
        }
        const std::string file_data = bytes.value_or(std::string{});
        if (dispute_id.has_value() && !dispute_id->empty()) {
            m_context->metrics().counter("files_retrieve_dispute_scoped_total").increment();
        }
        m_context->process_trackers().schedule(
            std::string{"file_retrieve_"} + *file_id,
            std::format("{{\"file_id\":\"{}\",\"provider_file_id\":\"{}\"}}",
                        *file_id, record->get_provider_file_id()),
            "0"
        );

        // ApplicationResponse::FileData: raw bytes plus the stored mime content type.
        res.set_status(interfaces::io::types::Status::OK);
        res.add_content_type(record->get_file_type());
        res.set_body(serde::Ser::serialize_raw("application/octet-stream", file_data));
        send();
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/files"}
            .add_route(
                Route{"/"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_files_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch files_create")
            )
            .add_route(
                Route{"/:file_id"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_files_delete(req, res, std::move(send));
                    })
                    .summary("Hyperswitch files_delete")
            )
            .add_route(
                Route{"/:file_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_files_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch files_retrieve")
            );
    }

private:
    /** @brief Upstream FilePurpose::DisputeEvidence discriminator. */
    [[nodiscard]] std::string_view purpose_dispute_evidence() const noexcept { return "dispute_evidence"; }

    /** @brief FileMetadataNew/Update availability states. */
    [[nodiscard]] std::string_view file_status_unavailable() const noexcept { return "UNAVAILABLE"; }
    [[nodiscard]] std::string_view file_status_available() const noexcept { return "AVAILABLE"; }

    /** @brief Local file upload provider (S3 substitute lives in ConfigStore). */
    [[nodiscard]] std::string_view file_upload_provider_router() const noexcept { return "router"; }
    [[nodiscard]] std::string_view file_storage_namespace() const noexcept { return "file_storage"; }

    /**
     * @brief generate_id(consts::ID_LENGTH = 20, "file") substitute.
     * @note IdGenerator::payment_id yields a 24-char hex suffix; trim it to the upstream
     * 20-character nanoid-shaped tail so the wire id keeps the same shape and length.
     */
    [[nodiscard]] std::string generate_file_id() const
    {
        constexpr std::size_t prefix_length{5};
        constexpr std::size_t id_length{20};
        const std::string raw = m_context->ids().payment_id("file_");
        return raw.substr(0, prefix_length + id_length);
    }

    /** @brief Deterministic substitute for the `infer` crate's mime detection. */
    [[nodiscard]] std::string infer_mime_type(std::string_view file_name, std::string_view declared) const
    {
        if (!declared.empty() && declared.find('/') != std::string_view::npos) {
            return std::string{declared};
        }
        const std::size_t dot = file_name.rfind('.');
        if (dot != std::string_view::npos) {
            const std::string extension = to_lower(file_name.substr(dot + 1));
            if (extension == "pdf") { return "application/pdf"; }
            if (extension == "png") { return "image/png"; }
            if (extension == "jpg" || extension == "jpeg") { return "image/jpeg"; }
            if (extension == "gif") { return "image/gif"; }
            if (extension == "csv") { return "text/csv"; }
            if (extension == "txt") { return "text/plain"; }
            if (extension == "json") { return "application/json"; }
            if (extension == "xml") { return "application/xml"; }
            if (extension == "zip") { return "application/zip"; }
        }
        return declared.empty() ? std::string{"application/octet-stream"} : std::string{declared};
    }

    /** @brief mime::Mime parse guard used by files_retrieve_core. */
    [[nodiscard]] bool is_valid_mime(std::string_view value) const noexcept
    {
        const std::size_t slash = value.find('/');
        return slash != std::string_view::npos && slash > 0 && slash + 1 < value.size();
    }

    /**
     * @brief Deterministic substitute for connector.validate_file_upload.
     * @returns An error when local validation fails; a nullopt when the file is acceptable.
     */
    [[nodiscard]] std::optional<Error> validate_file_upload(
        std::string_view merchant_id,
        std::string_view dispute_id,
        std::string_view file_type,
        std::int64_t file_size
    ) const
    {
        constexpr std::int64_t max_file_size{25LL * 1024 * 1024};
        if (file_size <= 0 || file_size > max_file_size) {
            return Builder{}(Kind::VALIDATION_FAILED, "file validation failed: invalid file size");
        }
        if (!is_valid_mime(file_type)) {
            return Builder{}(Kind::VALIDATION_FAILED, "file validation failed: invalid file type");
        }

        auto connector = m_context->connector_router().select(merchant_id);
        if (!connector) {
            // No connector registered: the deterministic local validation above is authoritative.
            m_context->metrics().counter("files_validation_local_total").increment();
            return std::nullopt;
        }
        app::connector::ConnectorRequest request{};
        request.set_operation("validate_file_upload");
        request.set_merchant_id(std::string{merchant_id});
        request.set_resource_id(std::string{dispute_id});
        request.set_raw_payload(std::format("{{\"purpose\":\"{}\",\"file_type\":\"{}\",\"file_size\":{}}}",
                                            purpose_dispute_evidence(), file_type, file_size));
        const auto response = connector->check_feature_matrix(request);
        m_context->metrics().counter("files_validation_connector_total").increment();
        if (response.get_retryable()) {
            return Builder{}(Kind::CONNECTOR_ERROR, "file validation connector failure");
        }
        return std::nullopt;
    }

    /** @brief Merchant scope used by upstream's processor-merchant-id metadata lookups. */
    [[nodiscard]] std::string scope_merchant_id(
        interfaces::io::IRequest& req,
        const app::auth::AuthContext& auth_context
    ) const
    {
        const auto header_merchant_id = app::http::merchant_id_from_header{}(req);
        (void)app::http::org_id_from_header{}(req);
        return header_merchant_id.empty() ? auth_context.merchant_id() : header_merchant_id;
    }

    /** @brief Profile id attached to file metadata (upstream resolves it during upload). */
    [[nodiscard]] std::string scope_profile_id(
        interfaces::io::IRequest& req,
        const app::auth::AuthContext& auth_context
    ) const
    {
        const auto header_profile_id = app::http::HeaderReader{}.get(req, "x-profile-id");
        return header_profile_id.empty() ? auth_context.profile_id() : header_profile_id;
    }

    /** @brief Extract the :file_id segment from the "/files/{file_id}" v1 routes. */
    [[nodiscard]] std::optional<std::string> file_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        for (std::size_t i{0}; i < segments.count(); ++i) {
            if (segments.at(i).value_or(std::string_view{}) != "files") {
                continue;
            }
            for (std::size_t j{i + 1}; j < segments.count(); ++j) {
                const auto candidate = segments.at(j).value_or(std::string_view{});
                if (!candidate.empty()) {
                    return std::string{candidate};
                }
            }
        }
        return std::nullopt;
    }

    /**
     * @brief Processor-merchant-id scoping substitute for file metadata lookups.
     * @note The repository keys files by file_id alone, so scope is enforced by checking the
     * persisted merchant_id (or an unscoped legacy record) after the natural-key lookup.
     */
    [[nodiscard]] std::optional<FileRecord> find_file_for_scope(
        std::string_view merchant_id,
        std::string_view file_id
    ) const
    {
        const auto in_scope = [merchant_id](const FileRecord& candidate)
        {
            return candidate.get_merchant_id().empty() || std::string_view{candidate.get_merchant_id()} == merchant_id;
        };
        const auto exact = m_context->repository().find_file_by_string(file_id);
        if (exact && in_scope(*exact)) {
            return exact;
        }
        for (const auto& candidate : m_context->repository().list_files()) {
            if (candidate.get_file_id() == file_id && in_scope(candidate)) {
                return candidate;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string to_lower(std::string_view value) const
    {
        std::string out{};
        out.reserve(value.size());
        for (const char character : value) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
        }
        return out;
    }

    [[nodiscard]] std::string now_string() const
    {
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count());
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        Error error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::files
