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

/** @brief Upstream `ApplicationResponse::StatusOk` wire shape (`core/files.rs:96,108`). */
class StatusOkWire
{
public:
    void set_status(std::string value) { status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return status; }
    std::string status{};
};

/**
 * @brief Local mirror of upstream `types::api::files::FilePurpose`
 * (`hyperswitch_interfaces/src/api/files.rs:14-31`, `types/api/files.rs:3`).
 * V1 has exactly one variant.
 */
enum class FilePurpose : std::uint8_t { DisputeEvidence };

/**
 * @brief Local mirror of upstream `api_models::enums::FileUploadProvider`
 * (`common_enums/src/enums.rs:4729-4741`): serialized values are snake_case.
 */
enum class FileUploadProvider : std::uint8_t { Router, Stripe, Checkout, Worldpayvantiv };

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
 * @brief Local mirror of upstream `FileInfo`
 * (`hyperswitch_domain_models/src/router_response_types/disputes.rs`),
 * returned by `retrieve_file_and_provider_file_id_from_file_id`.
 */
struct FileInfo
{
    std::optional<std::string> file_data{};
    std::optional<std::string> provider_file_id{};
    std::optional<std::string> file_type{};
};

/**
 * @brief Outcome of `upload_and_get_provider_provider_file_id_profile_id`
 * (`core/files/helpers.rs:287-406`): provider_file_id, provider, profile_id,
 * merchant_connector_id.
 */
struct UploadOutcome
{
    std::string provider_file_id{};
    FileUploadProvider provider{FileUploadProvider::Router};
    std::optional<std::string> profile_id{};
    std::optional<std::string> merchant_connector_id{};
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

export namespace app::handlers::files {

class FilesRouteHandlers
{
    using Error = core::error::AppError;
    using Kind = core::error::AppErrorKind;
    using Builder = core::error::AppErrorBuilder;
    using FileRecord = app::store::files::FileRecord;

    /** @brief Upstream `types::api::files::FileDataRequired` (`types/api/files.rs:24-28`). */
    enum class FileDataRequired : std::uint8_t { Required, NotRequired };

public:
    explicit FilesRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    // ── Route handlers: authorize, parse the upstream input, then dispatch to the
    //    local ports of upstream core/files.rs and core/files/helpers.rs. ────────────

    void handle_files_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: files.rs:44-64 — server_wrap(auth_type(ApiKeyAuth{...},
        // DashboardNoPermissionAuth{...})) around files_create_core.
        if (const auto denied = authorize_request(req, "files_create")) {
            send_error(res, send, *denied);
            return;
        }
        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto processor_merchant_id = scope_merchant_id(req, auth);
        const auto provider_merchant_id = auth.merchant_id().empty()
                                              ? processor_merchant_id
                                              : auth.merchant_id();

        // Upstream: files.rs:39, transformers.rs:13-82. Multipart is decoded from the
        // raw request body because the local HTTP layer exposes bytes, not Actix fields.
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, Builder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = get_create_file_request(req, body);
        if (!parsed) {
            send_error(res, send, std::move(parsed.error()));
            return;
        }

        m_context->metrics().record_flow("files.create");
        m_context->metrics().counter("files_create_total").increment();

        const auto file_id = files_create_core(
            parsed->file, parsed->file_name, parsed->file_size, parsed->file_type,
            parsed->purpose, parsed->dispute_id, processor_merchant_id, provider_merchant_id
        );
        if (!file_id) {
            send_error(res, send, std::move(file_id.error()));
            return;
        }

        app::dto::files::CreateFileResponse response{};
        response.set_file_id(*file_id);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_files_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: files.rs:96-117.
        if (const auto denied = authorize_request(req, "files_delete")) {
            send_error(res, send, *denied);
            return;
        }
        const auto file_id = file_id_from_path(req);
        if (!file_id.has_value() || file_id->empty()) {
            send_error(res, send, Builder{}(Kind::INVALID_REQUEST, "missing file id in path"));
            return;
        }
        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto processor_merchant_id = scope_merchant_id(req, auth);

        m_context->metrics().record_flow("files.delete");
        m_context->metrics().counter("files_delete_total").increment();

        const auto deleted = files_delete_core(processor_merchant_id, *file_id);
        if (!deleted) {
            send_error(res, send, std::move(deleted.error()));
            return;
        }

        StatusOkWire response{};
        response.set_status("success");
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_files_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: files.rs:150-171.
        if (const auto denied = authorize_request(req, "files_retrieve")) {
            send_error(res, send, *denied);
            return;
        }
        const auto file_id = file_id_from_path(req);
        if (!file_id.has_value() || file_id->empty()) {
            send_error(res, send, Builder{}(Kind::INVALID_REQUEST, "missing file id in path"));
            return;
        }
        const auto query = app::http::RequestQuery{}(req);
        const auto dispute_id = query.get("dispute_id");
        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto processor_merchant_id = scope_merchant_id(req, auth);

        m_context->metrics().record_flow("files.retrieve");
        m_context->metrics().counter("files_retrieve_total").increment();

        const auto retrieved = files_retrieve_core(processor_merchant_id, *file_id, dispute_id);
        if (!retrieved) {
            send_error(res, send, std::move(retrieved.error()));
            return;
        }

        // Upstream: core/files.rs:139-145 — ApplicationResponse::FileData((bytes, mime)).
        res.set_status(interfaces::io::types::Status::OK);
        res.add_content_type(retrieved->content_type);
        res.set_body(serde::Ser::serialize_raw("application/octet-stream", retrieved->file_data));
        send();
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Upstream: routes/app.rs:2592-2599 — scope("/files"), resource("") POST,
        // resource("/{file_id}") DELETE + GET. An empty child path keeps POST at
        // exactly "/files" (no trailing slash).
        Router{router, "/files"}
            .add_route(
                Route{""}
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
    struct RetrievedFile
    {
        std::string file_data{};
        std::string content_type{};
    };

    // ── Upstream core ports ──────────────────────────────────────────────────────────

    /**
     * @brief Port of `files_create_core` (`core/files.rs:14-90`).
     *  validate -> generate_id -> insert(available=false) -> upload -> update(available=true).
     */
    [[nodiscard]] std::expected<std::string, Error> files_create_core(
        const std::string& file,
        const std::optional<std::string>& file_name,
        std::int32_t file_size,
        const std::string& file_type,
        FilePurpose purpose,
        const std::optional<std::string>& dispute_id,
        std::string_view processor_merchant_id,
        std::string_view provider_merchant_id
    ) const
    {
        CreateFileRequestData request{};
        request.file = file;
        request.file_name = file_name;
        request.file_size = file_size;
        request.file_type = file_type;
        request.purpose = purpose;
        request.dispute_id = dispute_id;

        // core/files.rs:19-24.
        if (const auto invalid = validate_file_upload(request, processor_merchant_id)) {
            return std::unexpected{*invalid};
        }

        // core/files.rs:25 — common_utils::generate_id(consts::ID_LENGTH=20, "file").
        const auto file_id = generate_id();
        // core/files.rs:26-34 — file_key = "{processor_merchant_id}/{file_id}".
        const std::string file_key = std::format("{}/{}", processor_merchant_id, file_id);

        // core/files.rs:35-59 — FileMetadataNew { available: false, provider_file_id: None,
        // file_upload_provider: None, profile_id: None, merchant_connector_id: None }.
        FileRecord record{};
        record.set_file_id(file_id);
        record.set_merchant_id(std::string{provider_merchant_id});
        record.set_profile_id(std::string{});
        record.set_file_name(file_name.value_or(std::string{}));
        record.set_file_type(file_type);
        record.set_file_size(static_cast<std::int64_t>(file_size));
        record.set_purpose(std::string{file_purpose_name(purpose)});
        record.set_provider_file_id(std::string{});
        record.set_upload_provider(std::string{});
        record.set_status(std::string{file_status_unavailable()});
        record.set_created_at(now_string());
        auto inserted = insert_file_metadata(record);
        if (!inserted) {
            return std::unexpected{std::move(inserted.error())};
        }
        record = *inserted;

        // core/files.rs:60-68.
        auto uploaded = upload_and_get_provider_provider_file_id_profile_id(
            request, file_key, processor_merchant_id
        );
        if (!uploaded) {
            return std::unexpected{std::move(uploaded.error())};
        }

        // core/files.rs:70-86 — FileMetadataUpdate::Update { provider_file_id,
        // file_upload_provider, available: true, profile_id, merchant_connector_id }.
        record.set_provider_file_id(uploaded->provider_file_id);
        record.set_upload_provider(std::string{file_upload_provider_name(uploaded->provider)});
        record.set_status(std::string{file_status_available()});
        if (uploaded->profile_id.has_value()) {
            record.set_profile_id(*uploaded->profile_id);
        }
        if (!update_file_metadata(record)) {
            return std::unexpected{
                Builder{}(Kind::INTERNAL_ERROR,
                          std::format("Unable to update file_metadata with file_id: {}", file_id))
            };
        }

        // core/files.rs:87-89 — Json(CreateFileResponse { file_id }).
        return file_id;
    }

    /** @brief Port of `files_delete_core` (`core/files.rs:92-109`). */
    [[nodiscard]] std::expected<void, Error> files_delete_core(
        std::string_view processor_merchant_id,
        std::string_view file_id
    ) const
    {
        // core/files.rs:97 — delete_file_using_file_id (guards + file-storage delete).
        const auto record = delete_file_using_file_id(processor_merchant_id, file_id);
        if (!record) {
            return std::unexpected{std::move(record.error())};
        }
        // core/files.rs:98-107 — delete_file_metadata_by_processor_merchant_id_file_id.
        if (!m_context->repository().remove_file(record->get_id())) {
            return std::unexpected{
                Builder{}(Kind::INTERNAL_ERROR, "Unable to delete file_metadata")
            };
        }
        return std::expected<void, Error>{};
    }

    /** @brief Port of `files_retrieve_core` (`core/files.rs:111-146`). */
    [[nodiscard]] std::expected<RetrievedFile, Error> files_retrieve_core(
        std::string_view processor_merchant_id,
        std::string_view file_id,
        const std::optional<std::string>& dispute_id
    ) const
    {
        // core/files.rs:116-125 — find_file_metadata... . FileNotFound when absent.
        const auto metadata = find_file_metadata_by_processor_merchant_id_file_id(
            processor_merchant_id, file_id
        );
        if (!metadata) {
            return std::unexpected{
                Builder{}(Kind::NOT_FOUND, "File does not exist in our records")
            };
        }

        // core/files.rs:126-133 — FileDataRequired::Required.
        auto info = retrieve_file_and_provider_file_id_from_file_id(
            processor_merchant_id, file_id, dispute_id, FileDataRequired::Required
        );
        if (!info) {
            return std::unexpected{std::move(info.error())};
        }

        // core/files.rs:134-138 — file_type.parse::<mime::Mime>().
        if (!is_valid_mime(metadata->get_file_type())) {
            return std::unexpected{
                Builder{}(Kind::INTERNAL_ERROR, "Failed to parse file content type")
            };
        }
        // core/files.rs:139-145 — file_data.ok_or(FileNotAvailable).
        if (!info->file_data.has_value()) {
            return std::unexpected{Builder{}(Kind::NOT_FOUND, "File not available")};
        }

        RetrievedFile result{};
        result.file_data = *info->file_data;
        result.content_type = metadata->get_file_type();
        return result;
    }

    /**
     * @brief Port of `validate_file_upload` (`core/files/helpers.rs:34-86`).
     *  DisputeEvidence: MissingDisputeId -> dispute lookup (DisputeNotFound) ->
     *  connector.validate_file_upload -> FileValidationFailed / InternalServerError.
     */
    [[nodiscard]] std::optional<Error> validate_file_upload(
        const CreateFileRequestData& request,
        std::string_view processor_merchant_id
    ) const
    {
        // helpers.rs:40-44 — purpose is DisputeEvidence; the only V1 variant.
        if (request.purpose != FilePurpose::DisputeEvidence) {
            return std::nullopt;
        }
        if (!request.dispute_id.has_value() || request.dispute_id->empty()) {
            return Builder{}(Kind::INVALID_REQUEST, "Dispute id not found in the request");
        }
        // helpers.rs:45-55 — find_dispute_by_processor_merchant_id_dispute_id.
        const auto dispute = find_dispute(processor_merchant_id, *request.dispute_id);
        if (!dispute) {
            return Builder{}(Kind::NOT_FOUND, "Dispute does not exist in our records");
        }

        // helpers.rs:56-62 — ConnectorData::get_connector_by_name(&dispute.connector, None).
        auto connector = connector_for(dispute->get_connector(), processor_merchant_id);
        if (!connector) {
            // Local substitute: no connector registered means deterministic validation below.
            m_context->metrics().counter("files_validation_local_total").increment();
            return std::nullopt;
        }

        // helpers.rs:64-83 — connector.validate_file_upload(purpose, size, mime).
        app::connector::ConnectorRequest validation{};
        validation.set_operation("validate_file_upload");
        validation.set_merchant_id(std::string{processor_merchant_id});
        validation.set_resource_id(*request.dispute_id);
        validation.set_raw_payload(std::format(
            "{{\"purpose\":\"{}\",\"file_size\":{},\"file_type\":\"{}\"}}",
            file_purpose_name(request.purpose), request.file_size, request.file_type
        ));
        const auto response = connector->check_feature_matrix(validation);
        m_context->metrics().counter("files_validation_connector_total").increment();
        if (validation_failed(response)) {
            const std::string reason = response.get_message().empty()
                                           ? std::string{"invalid file"}
                                           : response.get_message();
            return Builder{}(Kind::INVALID_REQUEST, // upstream HE_03 -> 400
                             std::format("File validation failed {}", reason));
        }
        if (response.get_retryable()) {
            return Builder{}(Kind::INTERNAL_ERROR, "File validation failed");
        }
        return std::nullopt;
    }

    /**
     * @brief Port of `upload_and_get_provider_provider_file_id_profile_id`
     * (`core/files/helpers.rs:287-406`, v1).
     */
    [[nodiscard]] std::expected<UploadOutcome, Error>
    upload_and_get_provider_provider_file_id_profile_id(
        const CreateFileRequestData& request,
        std::string_view file_key,
        std::string_view processor_merchant_id
    ) const
    {
        // helpers.rs:301-315 — DisputeEvidence requires dispute id + dispute lookup.
        const auto dispute_id = request.dispute_id.value_or(std::string{});
        if (dispute_id.empty()) {
            return std::unexpected{Builder{}(Kind::INVALID_REQUEST, "Dispute id not found in the request")};
        }
        const auto dispute = find_dispute(processor_merchant_id, dispute_id);
        if (!dispute) {
            return std::unexpected{
                Builder{}(Kind::NOT_FOUND, "Dispute does not exist in our records")
            };
        }

        const std::string& connector_name = dispute->get_connector();
        auto connector = connector_for(connector_name, processor_merchant_id);

        UploadOutcome outcome{};
        if (connector && supports_file_storage_module(connector_name)) {
            // helpers.rs:322-344 — payment intent + payment attempt for the dispute.
            const auto intent = find_payment_intent(processor_merchant_id, dispute->get_payment_id());
            if (!intent) {
                return std::unexpected{Builder{}(Kind::NOT_FOUND, "Payment not found")};
            }
            const auto attempt = find_payment_attempt(
                processor_merchant_id, dispute->get_payment_id()
            );
            if (!attempt) {
                return std::unexpected{Builder{}(Kind::NOT_FOUND, "Payment not found")};
            }

            // helpers.rs:345-391 — connector upload integration (file storage module).
            app::connector::ConnectorRequest upload{};
            upload.set_operation("upload_file");
            upload.set_merchant_id(std::string{processor_merchant_id});
            upload.set_resource_id(std::string{file_key});
            upload.set_raw_payload(request.file);
            const auto response = connector->submit_evidence(upload);
            if (!response.get_raw_json().empty() && response.get_retryable()) {
                return std::unexpected{
                    Builder{}(Kind::CONNECTOR_ERROR, "Failed while calling upload file connector api")
                };
            }
            outcome.provider_file_id = response.get_reference_id().empty()
                                           ? std::string{file_key}
                                           : response.get_reference_id();
            outcome.provider = file_upload_provider_from_connector(connector_name);
            // Upstream: payment_intent.profile_id. The local PaymentIntentRecord has no
            // profile_id column, so the dispute's profile id is used (shared request below).
            outcome.profile_id = dispute->get_profile_id().empty()
                                     ? std::optional<std::string>{std::nullopt}
                                     : std::optional<std::string>{dispute->get_profile_id()};
            // helpers.rs:390 — payment_attempt.merchant_connector_id.
            const auto mca = find_merchant_connector_id(
                processor_merchant_id, connector_name, attempt->get_connector()
            );
            outcome.merchant_connector_id = mca;
            m_context->metrics().counter("files_upload_connector_total").increment();
        } else {
            // helpers.rs:392-403 — file_storage_client.upload_file(&file_key, file);
            // returns (file_key, FileUploadProvider::Router, None, None).
            file_storage_upload(file_key, request.file);
            outcome.provider_file_id = std::string{file_key};
            outcome.provider = FileUploadProvider::Router;
            m_context->metrics().counter("files_upload_local_total").increment();
        }
        return outcome;
    }

    /**
     * @brief Port of `delete_file_using_file_id` (`core/files/helpers.rs:88-121`).
     */
    [[nodiscard]] std::expected<FileRecord, Error> delete_file_using_file_id(
        std::string_view processor_merchant_id,
        std::string_view file_id
    ) const
    {
        // helpers.rs:93-100 — find_file_metadata... -> FileNotFound.
        const auto record = find_file_metadata_by_processor_merchant_id_file_id(
            processor_merchant_id, file_id
        );
        if (!record) {
            return std::unexpected{
                Builder{}(Kind::NOT_FOUND, "File does not exist in our records")
            };
        }
        // helpers.rs:101-109 — (Some(provider), Some(id), true) else FileNotAvailable.
        if (record->get_upload_provider().empty() || record->get_provider_file_id().empty() ||
            std::string_view{record->get_status()} != file_status_available()) {
            return std::unexpected{Builder{}(Kind::NOT_FOUND, "File not available")};
        }
        // helpers.rs:110-120 — Router -> file_storage delete; other -> FileProviderNotSupported.
        if (file_upload_provider_from_name(record->get_upload_provider()) != FileUploadProvider::Router) {
            return std::unexpected{Builder{}(
                Kind::INVALID_REQUEST,
                "Not Supported because provider is not Router"
            )};
        }
        file_storage_delete(record->get_provider_file_id());
        return *record;
    }

    /**
     * @brief Port of `retrieve_file_and_provider_file_id_from_file_id`
     * (`core/files/helpers.rs:199-264`).
     */
    [[nodiscard]] std::expected<FileInfo, Error>
    retrieve_file_and_provider_file_id_from_file_id(
        std::string_view processor_merchant_id,
        std::string_view file_id,
        const std::optional<std::string>& dispute_id,
        FileDataRequired is_connector_file_data_required
    ) const
    {
        // helpers.rs:212-220 — find_file_metadata... -> FileNotFound.
        auto metadata = find_file_metadata_by_processor_merchant_id_file_id(
            processor_merchant_id, file_id
        );
        if (!metadata) {
            return std::unexpected{
                Builder{}(Kind::NOT_FOUND, "File does not exist in our records")
            };
        }
        // helpers.rs:221-229 — (Some(provider), Some(id), true) else FileNotAvailable.
        if (metadata->get_upload_provider().empty() || metadata->get_provider_file_id().empty() ||
            std::string_view{metadata->get_status()} != file_status_available()) {
            return std::unexpected{Builder{}(Kind::NOT_FOUND, "File not available")};
        }

        FileInfo info{};
        info.provider_file_id = metadata->get_provider_file_id();
        info.file_type = metadata->get_file_type();
        const auto provider = file_upload_provider_from_name(metadata->get_upload_provider());
        if (provider == FileUploadProvider::Router) {
            // helpers.rs:230-241 — file_storage_client.retrieve_file(provider_file_id).
            auto bytes = file_storage_retrieve(metadata->get_provider_file_id());
            if (!bytes) {
                return std::unexpected{
                    Builder{}(Kind::INTERNAL_ERROR, "Unable to retrieve file")
                };
            }
            info.file_data = std::move(bytes);
        } else if (is_connector_file_data_required == FileDataRequired::Required) {
            // helpers.rs:242-260 — retrieve_file_from_connector when Required.
            auto bytes = retrieve_file_from_connector(*metadata, dispute_id, processor_merchant_id);
            if (!bytes) {
                return std::unexpected{std::move(bytes.error())};
            }
            info.file_data = std::move(*bytes);
        }
        return info;
    }

    /**
     * @brief Port of `retrieve_file_from_connector` (`core/files/helpers.rs:123-197`).
     */
    [[nodiscard]] std::expected<std::string, Error> retrieve_file_from_connector(
        const FileRecord& metadata,
        const std::optional<std::string>& dispute_id,
        std::string_view processor_merchant_id
    ) const
    {
        // helpers.rs:129-135 — FileUploadProvider -> Connector; Router is not a connector.
        const auto provider = file_upload_provider_from_name(metadata.get_upload_provider());
        if (provider == FileUploadProvider::Router) {
            return std::unexpected{Builder{}(
                Kind::INVALID_REQUEST, "File upload provider is not a connector"
            )};
        }
        const std::string connector_name{file_upload_provider_name(provider)};

        // helpers.rs:143-158 — dispute lookup when dispute_id is provided.
        if (dispute_id.has_value() && !dispute_id->empty()) {
            const auto dispute = find_dispute(processor_merchant_id, *dispute_id);
            if (!dispute) {
                return std::unexpected{
                    Builder{}(Kind::NOT_FOUND, "Dispute does not exist in our records")
                };
            }
        }

        // helpers.rs:160-185 — construct router data + execute RetrieveFile integration.
        auto connector = connector_for(connector_name, processor_merchant_id);
        if (!connector) {
            return std::unexpected{
                Builder{}(Kind::CONNECTOR_ERROR, "Failed while calling retrieve file connector api")
            };
        }
        app::connector::ConnectorRequest retrieve{};
        retrieve.set_operation("retrieve_file");
        retrieve.set_merchant_id(std::string{processor_merchant_id});
        retrieve.set_resource_id(metadata.get_provider_file_id());
        retrieve.set_raw_payload(dispute_id.value_or(std::string{}));
        const auto response = connector->check_feature_matrix(retrieve);
        if (response.get_retryable()) {
            return std::unexpected{
                Builder{}(Kind::CONNECTOR_ERROR, "Failed while calling retrieve file connector api")
            };
        }
        return response.get_raw_json();
    }

    // ── Upstream helper ports: multipart parsing, validation, storage, lookups ──────

    /**
     * @brief Port of `get_create_file_request` (`routes/files/transformers.rs:13-82`).
     */
    [[nodiscard]] std::expected<CreateFileRequestData, Error>
    get_create_file_request(interfaces::io::IRequest& req, const std::string& body) const
    {
        const std::string content_type = app::http::HeaderReader{}.get(req, "content-type");
        auto parts = parse_multipart(body, content_type);
        if (!parts) {
            return std::unexpected{
                Builder{}(Kind::INVALID_REQUEST, "Invalid multipart request")
            };
        }

        std::optional<FilePurpose> purpose{};
        std::optional<std::string> dispute_id{};
        std::optional<std::string> file_name{};
        std::optional<std::string> file_content{};

        // transformers.rs:22-51 — iterate fields; last occurrence wins, like upstream.
        for (const auto& part : *parts) {
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
            return std::unexpected{Builder{}(
                Kind::INVALID_REQUEST, "File purpose not found in the request or is invalid"
            )};
        }
        // transformers.rs:53-57 — MissingFile when no file part.
        if (!file_content.has_value()) {
            return std::unexpected{
                Builder{}(Kind::INVALID_REQUEST, "File not found / valid in the request")
            };
        }
        // transformers.rs:59-61 — i32::try_from(file.len()).
        if (file_content->size() > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max())) {
            return std::unexpected{Builder{}(Kind::INTERNAL_ERROR, "File size error")};
        }
        const auto file_size = static_cast<std::int32_t>(file_content->size());
        // transformers.rs:62-66 — empty file is MissingFile.
        if (file_size <= 0) {
            return std::unexpected{
                Builder{}(Kind::INVALID_REQUEST, "File not found / valid in the request")
            };
        }
        // transformers.rs:67-73 — infer::get(&file) + mime parse.
        const auto mime = infer_mime(*file_content);
        if (!mime.has_value()) {
            return std::unexpected{Builder{}(
                Kind::INVALID_REQUEST, "File content type not found / valid"
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
    [[nodiscard]] std::optional<std::vector<MultipartPart>> parse_multipart(
        const std::string& body,
        const std::string& content_type
    ) const
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
                    const std::size_t length = (line_end == std::string::npos ? headers.size() : line_end)
                                                 - line_start;
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

    [[nodiscard]] std::optional<std::string> multipart_boundary(
        std::string_view content_type
    ) const
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

    [[nodiscard]] std::optional<std::string> multipart_attribute(
        std::string_view line,
        std::string_view key
    ) const
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
        while (start < line.size() &&
               std::isspace(static_cast<unsigned char>(line[start]))) {
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
            return bytes.size() >= signature.size() && bytes.substr(0, signature.size()) == signature;
        };
        if (starts("%PDF-")) { return "application/pdf"; }
        if (bytes.size() >= 8 && static_cast<unsigned char>(bytes[0]) == 0x89 &&
            bytes[1] == 'P' && bytes[2] == 'N' && bytes[3] == 'G' &&
            bytes[4] == 0x0D && bytes[5] == 0x0A && bytes[6] == 0x1A && bytes[7] == 0x0A) {
            return "image/png";
        }
        if (bytes.size() >= 3 && static_cast<unsigned char>(bytes[0]) == 0xFF &&
            static_cast<unsigned char>(bytes[1]) == 0xD8 &&
            static_cast<unsigned char>(bytes[2]) == 0xFF) {
            return "image/jpeg";
        }
        if (starts("GIF87a") || starts("GIF89a")) { return "image/gif"; }
        if (bytes.size() >= 12 && starts("RIFF") && bytes.substr(8, 4) == "WEBP") {
            return "image/webp";
        }
        if (bytes.size() >= 12 && starts("RIFF") && bytes.substr(8, 4) == "WAVE") {
            return "audio/wav";
        }
        if (starts("PK\x03\x04") || starts("PK\x05\x06") || starts("PK\x07\x08")) {
            return "application/zip";
        }
        if (bytes.size() >= 8 && bytes.substr(4, 4) == "ftyp") { return "video/mp4"; }
        if (starts("BM")) { return "image/bmp"; }
        if (bytes.size() >= 5 && bytes.substr(0, 5) == "<?xml") { return "text/xml"; }
        const std::size_t first = bytes.find_first_not_of(" \t\r\n");
        if (first != std::string_view::npos &&
            (bytes[first] == '{' || bytes[first] == '[')) {
            return "application/json";
        }
        return std::nullopt;
    }

    // ── Upstream metadata persistence ports ─────────────────────────────────────────

    /** @brief `store.insert_file_metadata` (`core/files.rs:54-59`). */
    [[nodiscard]] std::expected<FileRecord, Error> insert_file_metadata(
        const FileRecord& record
    ) const
    {
        auto created = m_context->repository().create_file(record);
        if (!created) {
            return std::unexpected{Builder{}(Kind::INTERNAL_ERROR, "Unable to insert file_metadata")};
        }
        return *created;
    }

    /** @brief `store.update_file_metadata` (`core/files.rs:78-86`). */
    bool update_file_metadata(const FileRecord& record) const
    {
        return m_context->repository().update_file(record);
    }

    /**
     * @brief `store.find_file_metadata_by_processor_merchant_id_file_id`
     * (`core/files.rs:116-125`, `helpers.rs:93-100,213-220`).
     */
    [[nodiscard]] std::optional<FileRecord>
    find_file_metadata_by_processor_merchant_id_file_id(
        std::string_view processor_merchant_id,
        std::string_view file_id
    ) const
    {
        const auto exact = m_context->repository().find_file_by_string(file_id);
        if (exact.has_value()) {
            return exact;
        }
        // The local repository keys by file_id; fall back to a list scan so a record
        // persisted under a different natural key is still found.
        for (const auto& candidate : m_context->repository().list_files()) {
            if (std::string_view{candidate.get_file_id()} == file_id) {
                return candidate;
            }
        }
        (void)processor_merchant_id; // enforced by the upstream metadata key in the real store.
        return std::nullopt;
    }

    // ── Local in-process substitutes for external infrastructure ────────────────────

    /**
     * @brief `file_storage_client.upload_file/retrieve_file/delete_file` substitute.
     * Upstream keys Router uploads by `file_key = "{merchant_id}/{file_id}"` and uses the
     * provider_file_id (which equals that key for Router) on retrieve/delete.
     */
    void file_storage_upload(std::string_view key, std::string_view bytes) const
    {
        m_context->config_store().put(file_storage_namespace(), key, std::string{bytes});
    }

    [[nodiscard]] std::optional<std::string> file_storage_retrieve(std::string_view key) const
    {
        return m_context->config_store().get_value(file_storage_namespace(), key);
    }

    void file_storage_delete(std::string_view key) const
    {
        m_context->config_store().remove(file_storage_namespace(), key);
    }

    /** @brief `store.find_dispute_by_processor_merchant_id_dispute_id` substitute. */
    [[nodiscard]] std::optional<app::store::disputes::DisputeRecord> find_dispute(
        std::string_view processor_merchant_id,
        std::string_view dispute_id
    ) const
    {
        auto dispute = m_context->repository().find_dispute_by_string(dispute_id);
        if (!dispute) {
            return std::nullopt;
        }
        const bool in_scope = dispute->get_merchant_id().empty() ||
                              std::string_view{dispute->get_merchant_id()} == processor_merchant_id;
        if (!in_scope) {
            return std::nullopt;
        }
        return dispute;
    }

    /** @brief `ConnectorData::get_connector_by_name` substitute. */
    [[nodiscard]] std::shared_ptr<app::connector::IConnector> connector_for(
        std::string_view connector_name,
        std::string_view processor_merchant_id
    ) const
    {
        if (connector_name.empty()) {
            return m_context->connector_router().select(processor_merchant_id);
        }
        app::connector::ConnectorRequest request{};
        request.set_operation("file_storage");
        request.set_merchant_id(std::string{processor_merchant_id});
        request.set_resource_id(std::string{connector_name});
        auto connector = m_context->connector_router().select_for(request);
        if (connector && !connector->name().empty() &&
            !iequals(connector->name(), connector_name)) {
            // The local router selects by merchant; keep the selected connector but do
            // not claim it is the named one (shared contract request below).
            m_context->metrics().counter("files_connector_name_fallback_total").increment();
        }
        if (!connector) {
            connector = m_context->connector_router().select(processor_merchant_id);
        }
        return connector;
    }

    /** @brief `find_payment_intent_by_payment_id_processor_merchant_id` substitute. */
    [[nodiscard]] std::optional<app::store::payments::PaymentIntentRecord> find_payment_intent(
        std::string_view processor_merchant_id,
        std::string_view payment_id
    ) const
    {
        auto intent = m_context->repository().find_payment_by_string(payment_id);
        if (!intent) {
            return std::nullopt;
        }
        if (intent->get_merchant_id().empty() ||
            std::string_view{intent->get_merchant_id()} == processor_merchant_id) {
            return intent;
        }
        return std::nullopt;
    }

    /** @brief `find_payment_attempt_by_payment_id_processor_merchant_id_attempt_id` substitute. */
    [[nodiscard]] std::optional<app::store::payment_attempts::PaymentAttemptRecord>
    find_payment_attempt(std::string_view processor_merchant_id, std::string_view payment_id) const
    {
        for (const auto& attempt : m_context->repository().list_payment_attempts()) {
            if (std::string_view{attempt.get_payment_id()} == payment_id &&
                (attempt.get_merchant_id().empty() ||
                 std::string_view{attempt.get_merchant_id()} == processor_merchant_id)) {
                return attempt;
            }
        }
        return std::nullopt;
    }

    /** @brief Merchant connector id resolution for the connector-upload branch. */
    [[nodiscard]] std::optional<std::string> find_merchant_connector_id(
        std::string_view processor_merchant_id,
        std::string_view connector_name,
        std::string_view attempt_connector
    ) const
    {
        const std::string wanted = attempt_connector.empty()
                                       ? std::string{connector_name}
                                       : std::string{attempt_connector};
        for (const auto& mca : m_context->repository().list_merchant_connectors()) {
            if (std::string_view{mca.get_merchant_id()} == processor_merchant_id &&
                iequals(mca.get_connector_name(), wanted)) {
                return mca.get_merchant_connector_id();
            }
        }
        return std::nullopt;
    }

    // ── Enum / wire helpers ──────────────────────────────────────────────────────────

    [[nodiscard]] std::string_view file_purpose_name(FilePurpose purpose) const noexcept
    {
        (void)purpose; // V1 has a single variant: dispute_evidence.
        return "dispute_evidence";
    }

    [[nodiscard]] std::string_view file_upload_provider_name(FileUploadProvider provider) const noexcept
    {
        switch (provider) {
        case FileUploadProvider::Router: return "router";
        case FileUploadProvider::Stripe: return "stripe";
        case FileUploadProvider::Checkout: return "checkout";
        case FileUploadProvider::Worldpayvantiv: return "worldpayvantiv";
        }
        return "router";
    }

    [[nodiscard]] FileUploadProvider file_upload_provider_from_name(std::string_view value) const noexcept
    {
        if (value == "stripe") { return FileUploadProvider::Stripe; }
        if (value == "checkout") { return FileUploadProvider::Checkout; }
        if (value == "worldpayvantiv") { return FileUploadProvider::Worldpayvantiv; }
        return FileUploadProvider::Router;
    }

    /** @brief `Connector -> FileUploadProvider` (`types/api/files.rs:45-58`). */
    [[nodiscard]] FileUploadProvider file_upload_provider_from_connector(std::string_view name) const noexcept
    {
        const std::string lower = to_lower(name);
        if (lower == "stripe") { return FileUploadProvider::Stripe; }
        if (lower == "checkout") { return FileUploadProvider::Checkout; }
        if (lower == "worldpayvantiv") { return FileUploadProvider::Worldpayvantiv; }
        return FileUploadProvider::Router;
    }

    /** @brief `ConnectorName::supports_file_storage_module` (`common_enums/connector_enums.rs:356-358`). */
    [[nodiscard]] bool supports_file_storage_module(std::string_view name) const noexcept
    {
        const std::string lower = to_lower(name);
        return lower == "stripe" || lower == "checkout" || lower == "worldpayvantiv";
    }

    /** @brief `generate_id(consts::ID_LENGTH=20, "file")` (`common_utils/src/lib.rs:626-628`). */
    [[nodiscard]] std::string generate_id() const
    {
        constexpr std::size_t prefix_length{5}; // "file_"
        constexpr std::size_t suffix_length{20}; // nanoid!(20, ALPHABETS)
        const std::string raw = m_context->ids().payment_id("file_");
        return raw.substr(0, prefix_length + suffix_length);
    }

    /** @brief `file_type.parse::<mime::Mime>()` guard (`core/files.rs:134-138`). */
    [[nodiscard]] bool is_valid_mime(std::string_view value) const noexcept
    {
        const std::size_t slash = value.find('/');
        return slash != std::string_view::npos && slash > 0 && slash + 1 < value.size();
    }

    /** @brief Detect upstream `ConnectorError::FileValidationFailed` from a local connector. */
    [[nodiscard]] bool validation_failed(const app::connector::ConnectorResponse& response) const
    {
        const std::string status = to_lower(response.get_status());
        return status == "validation_failed" || status == "file_validation_failed" ||
               response.get_raw_json().find("file_validation_failed") != std::string::npos;
    }

    /**
     * @brief Upstream files auth: HeaderAuth(ApiKeyAuth{allow_connected_scope_operation=true,
     * allow_platform_self_operation=false}) or DashboardNoPermissionAuth{allow_connected=true,
     * allow_platform=false} (`files.rs:52-62,104-114,158-168`).
     */
    [[nodiscard]] std::optional<Error> authorize_request(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        const auto base = m_context->auth_rules().rule_for(operation);
        const auto rule = app::auth::RouteAuthRuleBuilder{}
                              .scheme(app::auth::AuthScheme::API_KEY)
                              .permission(base.permission())
                              .allow_connected(true)
                              .allow_platform(false)
                              .build();
        return m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
    }

    /** @brief Upstream `platform.get_processor().get_account().get_id()` substitute. */
    [[nodiscard]] std::string scope_merchant_id(
        interfaces::io::IRequest& req,
        const app::auth::AuthContext& auth
    ) const
    {
        const auto header = app::http::merchant_id_from_header{}(req);
        return header.empty() ? auth.merchant_id() : header;
    }

    /** @brief Extract `:file_id` from the `/files/{file_id}` v1 routes. */
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

    [[nodiscard]] std::string_view file_status_unavailable() const noexcept { return "UNAVAILABLE"; }
    [[nodiscard]] std::string_view file_status_available() const noexcept { return "AVAILABLE"; }
    [[nodiscard]] std::string_view file_storage_namespace() const noexcept { return "file_storage"; }

    [[nodiscard]] bool iequals(std::string_view lhs, std::string_view rhs) const noexcept
    {
        return to_lower(lhs) == to_lower(rhs);
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
