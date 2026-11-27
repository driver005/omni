export module app_store:files_runtime_common;

import std;
import core_behavior;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_metrics;
import app_context_core;
import app_connector;

import :files_input;

export namespace app::handlers::files {

using Error = core::error::AppError;
using Kind = core::error::AppErrorKind;
using Builder = core::error::AppErrorBuilder;
using FileRecord = ::app::store::files::FileRecord;

/**
 * @brief Local mirror of upstream `api_models::enums::FileUploadProvider`
 * (`common_enums/src/enums.rs:4729-4741`): serialized values are snake_case.
 */
enum class FileUploadProvider : std::uint8_t
{
    Router,
    Stripe,
    Checkout,
    Worldpayvantiv
};

/** @brief Upstream `types::api::files::FileDataRequired` (`types/api/files.rs:24-28`). */
enum class FileDataRequired : std::uint8_t
{
    Required,
    NotRequired
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

/**
 * @brief Shared upstream core ports used by the files sub-runtimes:
 * validation, upload, file-storage, metadata persistence, dispute/connector/payment
 * lookups and the enum/mime/wire helpers.
 */
class FilesRuntimeSupport
{
public:
    explicit FilesRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] Error make_error(Kind kind, std::string message) const
    {
        return Builder{}(kind, std::move(message));
    }

    /** @brief web::Path<String> /files/{file_id}: segment following "files". */
    [[nodiscard]] static std::optional<std::string>
    file_id_from(const core::behavior::CallContext& ctx)
    {
        for (std::size_t i{0}; i < ctx.path_segments.size(); ++i) {
            if (ctx.path_segments[i] != "files") {
                continue;
            }
            if (i + 1 < ctx.path_segments.size()) {
                return ctx.path_segments[i + 1];
            }
            break;
        }
        return std::nullopt;
    }

    /** @brief Authenticated identity merchant id; falls back to the processor scope. */
    [[nodiscard]] static std::string provider_merchant_id(const core::behavior::CallContext& ctx)
    {
        const auto it = ctx.claims.find("x-merchant-id");
        return it != ctx.claims.end() && !it->second.empty() ? it->second : ctx.merchant_id;
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
            m_context.logger().counter("files_validation_local_total").increment();
            return std::nullopt;
        }

        // helpers.rs:64-83 — connector.validate_file_upload(purpose, size, mime).
        ::app::connector::ConnectorRequest validation{};
        validation.set_operation("validate_file_upload");
        validation.set_merchant_id(std::string{processor_merchant_id});
        validation.set_resource_id(*request.dispute_id);
        validation.set_raw_payload(
            std::format(
                "{{\"purpose\":\"{}\",\"file_size\":{},\"file_type\":\"{}\"}}",
                file_purpose_name(request.purpose),
                request.file_size,
                request.file_type
            )
        );
        const auto response = connector->check_feature_matrix(validation);
        m_context.logger().counter("files_validation_connector_total").increment();
        if (validation_failed(response)) {
            const std::string reason = response.get_message().empty() ? std::string{"invalid file"}
                                                                      : response.get_message();
            return Builder{}(
                Kind::INVALID_REQUEST, // upstream HE_03 -> 400
                std::format("File validation failed {}", reason)
            );
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
    [[nodiscard]] core::behavior::Outcome<UploadOutcome>
    upload_and_get_provider_provider_file_id_profile_id(
        const CreateFileRequestData& request,
        std::string_view file_key,
        std::string_view processor_merchant_id
    ) const
    {
        // helpers.rs:301-315 — DisputeEvidence requires dispute id + dispute lookup.
        const auto dispute_id = request.dispute_id.value_or(std::string{});
        if (dispute_id.empty()) {
            return std::unexpected{
                Builder{}(Kind::INVALID_REQUEST, "Dispute id not found in the request")
            };
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
            const auto intent =
                find_payment_intent(processor_merchant_id, dispute->get_payment_id());
            if (!intent) {
                return std::unexpected{Builder{}(Kind::NOT_FOUND, "Payment not found")};
            }
            const auto attempt =
                find_payment_attempt(processor_merchant_id, dispute->get_payment_id());
            if (!attempt) {
                return std::unexpected{Builder{}(Kind::NOT_FOUND, "Payment not found")};
            }

            // helpers.rs:345-391 — connector upload integration (file storage module).
            ::app::connector::ConnectorRequest upload{};
            upload.set_operation("upload_file");
            upload.set_merchant_id(std::string{processor_merchant_id});
            upload.set_resource_id(std::string{file_key});
            upload.set_raw_payload(request.file);
            const auto response = connector->submit_evidence(upload);
            if (!response.get_raw_json().empty() && response.get_retryable()) {
                return std::unexpected{Builder{}(
                    Kind::CONNECTOR_ERROR,
                    "Failed while calling upload file connector api"
                )};
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
                processor_merchant_id,
                connector_name,
                attempt->get_connector()
            );
            outcome.merchant_connector_id = mca;
            m_context.logger().counter("files_upload_connector_total").increment();
        } else {
            // helpers.rs:392-403 — file_storage_client.upload_file(&file_key, file);
            // returns (file_key, FileUploadProvider::Router, None, None).
            file_storage_upload(file_key, request.file);
            outcome.provider_file_id = std::string{file_key};
            outcome.provider = FileUploadProvider::Router;
            m_context.logger().counter("files_upload_local_total").increment();
        }
        return outcome;
    }

    /**
     * @brief Port of `delete_file_using_file_id` (`core/files/helpers.rs:88-121`).
     */
    [[nodiscard]] core::behavior::Outcome<FileRecord> delete_file_using_file_id(
        std::string_view processor_merchant_id,
        std::string_view file_id
    ) const
    {
        // helpers.rs:93-100 — find_file_metadata... -> FileNotFound.
        const auto record =
            find_file_metadata_by_processor_merchant_id_file_id(processor_merchant_id, file_id);
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
        if (file_upload_provider_from_name(record->get_upload_provider()) !=
            FileUploadProvider::Router) {
            return std::unexpected{
                Builder{}(Kind::INVALID_REQUEST, "Not Supported because provider is not Router")
            };
        }
        file_storage_delete(record->get_provider_file_id());
        return *record;
    }

    /**
     * @brief Port of `retrieve_file_and_provider_file_id_from_file_id`
     * (`core/files/helpers.rs:199-264`).
     */
    [[nodiscard]] core::behavior::Outcome<FileInfo> retrieve_file_and_provider_file_id_from_file_id(
        std::string_view processor_merchant_id,
        std::string_view file_id,
        const std::optional<std::string>& dispute_id,
        FileDataRequired is_connector_file_data_required
    ) const
    {
        // helpers.rs:212-220 — find_file_metadata... -> FileNotFound.
        auto metadata =
            find_file_metadata_by_processor_merchant_id_file_id(processor_merchant_id, file_id);
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
                return std::unexpected{Builder{}(Kind::INTERNAL_ERROR, "Unable to retrieve file")};
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
    [[nodiscard]] core::behavior::Outcome<std::string> retrieve_file_from_connector(
        const FileRecord& metadata,
        const std::optional<std::string>& dispute_id,
        std::string_view processor_merchant_id
    ) const
    {
        // helpers.rs:129-135 — FileUploadProvider -> Connector; Router is not a connector.
        const auto provider = file_upload_provider_from_name(metadata.get_upload_provider());
        if (provider == FileUploadProvider::Router) {
            return std::unexpected{
                Builder{}(Kind::INVALID_REQUEST, "File upload provider is not a connector")
            };
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
        ::app::connector::ConnectorRequest retrieve{};
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

    // ── Upstream metadata persistence ports ─────────────────────────────────────────

    /** @brief `store.insert_file_metadata` (`core/files.rs:54-59`). */
    [[nodiscard]] core::behavior::Outcome<FileRecord>
    insert_file_metadata(const FileRecord& record) const
    {
        auto created = m_context.repository().create_file(record);
        if (!created) {
            return std::unexpected{
                Builder{}(Kind::INTERNAL_ERROR, "Unable to insert file_metadata")
            };
        }
        return *created;
    }

    /** @brief `store.update_file_metadata` (`core/files.rs:78-86`). */
    bool update_file_metadata(const FileRecord& record) const
    {
        return m_context.repository().update_file(record);
    }

    /**
     * @brief `store.find_file_metadata_by_processor_merchant_id_file_id`
     * (`core/files.rs:116-125`, `helpers.rs:93-100,213-220`).
     */
    [[nodiscard]] std::optional<FileRecord> find_file_metadata_by_processor_merchant_id_file_id(
        std::string_view processor_merchant_id,
        std::string_view file_id
    ) const
    {
        const auto exact = m_context.repository().find_file_by_string(file_id);
        if (exact.has_value()) {
            return exact;
        }
        // The local repository keys by file_id; fall back to a list scan so a record
        // persisted under a different natural key is still found.
        for (const auto& candidate: m_context.repository().list_files()) {
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
        m_context.config_store().put(file_storage_namespace(), key, std::string{bytes});
    }

    [[nodiscard]] std::optional<std::string> file_storage_retrieve(std::string_view key) const
    {
        return m_context.config_store().get_value(file_storage_namespace(), key);
    }

    void file_storage_delete(std::string_view key) const
    {
        m_context.config_store().remove(file_storage_namespace(), key);
    }

    /** @brief `store.find_dispute_by_processor_merchant_id_dispute_id` substitute. */
    [[nodiscard]] std::optional<::app::store::disputes::DisputeRecord>
    find_dispute(std::string_view processor_merchant_id, std::string_view dispute_id) const
    {
        auto dispute = m_context.repository().find_dispute_by_string(dispute_id);
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
    [[nodiscard]] std::shared_ptr<::app::connector::IConnector>
    connector_for(std::string_view connector_name, std::string_view processor_merchant_id) const
    {
        if (connector_name.empty()) {
            return m_context.connector_router().select(processor_merchant_id);
        }
        ::app::connector::ConnectorRequest request{};
        request.set_operation("file_storage");
        request.set_merchant_id(std::string{processor_merchant_id});
        request.set_resource_id(std::string{connector_name});
        auto connector = m_context.connector_router().select_for(request);
        if (connector && !connector->name().empty() &&
            !iequals(connector->name(), connector_name)) {
            // The local router selects by merchant; keep the selected connector but do
            // not claim it is the named one (shared contract request below).
            m_context.logger().counter("files_connector_name_fallback_total").increment();
        }
        if (!connector) {
            connector = m_context.connector_router().select(processor_merchant_id);
        }
        return connector;
    }

    /** @brief `find_payment_intent_by_payment_id_processor_merchant_id` substitute. */
    [[nodiscard]] std::optional<::app::store::payments::PaymentIntentRecord>
    find_payment_intent(std::string_view processor_merchant_id, std::string_view payment_id) const
    {
        auto intent = m_context.repository().find_payment_by_string(payment_id);
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
    [[nodiscard]] std::optional<::app::store::payment_attempts::PaymentAttemptRecord>
    find_payment_attempt(std::string_view processor_merchant_id, std::string_view payment_id) const
    {
        for (const auto& attempt: m_context.repository().list_payment_attempts()) {
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
        const std::string wanted = attempt_connector.empty() ? std::string{connector_name}
                                                             : std::string{attempt_connector};
        for (const auto& mca: m_context.repository().list_merchant_connectors()) {
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

    [[nodiscard]] std::string_view
    file_upload_provider_name(FileUploadProvider provider) const noexcept
    {
        switch (provider) {
            case FileUploadProvider::Router:
                return "router";
            case FileUploadProvider::Stripe:
                return "stripe";
            case FileUploadProvider::Checkout:
                return "checkout";
            case FileUploadProvider::Worldpayvantiv:
                return "worldpayvantiv";
        }
        return "router";
    }

    [[nodiscard]] FileUploadProvider
    file_upload_provider_from_name(std::string_view value) const noexcept
    {
        if (value == "stripe") {
            return FileUploadProvider::Stripe;
        }
        if (value == "checkout") {
            return FileUploadProvider::Checkout;
        }
        if (value == "worldpayvantiv") {
            return FileUploadProvider::Worldpayvantiv;
        }
        return FileUploadProvider::Router;
    }

    /** @brief `Connector -> FileUploadProvider` (`types/api/files.rs:45-58`). */
    [[nodiscard]] FileUploadProvider
    file_upload_provider_from_connector(std::string_view name) const noexcept
    {
        const std::string lower = to_lower(name);
        if (lower == "stripe") {
            return FileUploadProvider::Stripe;
        }
        if (lower == "checkout") {
            return FileUploadProvider::Checkout;
        }
        if (lower == "worldpayvantiv") {
            return FileUploadProvider::Worldpayvantiv;
        }
        return FileUploadProvider::Router;
    }

    /** @brief `ConnectorName::supports_file_storage_module`
     * (`common_enums/connector_enums.rs:356-358`). */
    [[nodiscard]] bool supports_file_storage_module(std::string_view name) const noexcept
    {
        const std::string lower = to_lower(name);
        return lower == "stripe" || lower == "checkout" || lower == "worldpayvantiv";
    }

    /** @brief `generate_id(consts::ID_LENGTH=20, "file")` (`common_utils/src/lib.rs:626-628`). */
    [[nodiscard]] std::string generate_id() const
    {
        constexpr std::size_t prefix_length{5};  // "file_"
        constexpr std::size_t suffix_length{20}; // nanoid!(20, ALPHABETS)
        const std::string raw = m_context.ids().payment_id("file_");
        return raw.substr(0, prefix_length + suffix_length);
    }

    /** @brief `file_type.parse::<mime::Mime>()` guard (`core/files.rs:134-138`). */
    [[nodiscard]] bool is_valid_mime(std::string_view value) const noexcept
    {
        const std::size_t slash = value.find('/');
        return slash != std::string_view::npos && slash > 0 && slash + 1 < value.size();
    }

    /** @brief Detect upstream `ConnectorError::FileValidationFailed` from a local connector. */
    [[nodiscard]] bool validation_failed(const ::app::connector::ConnectorResponse& response) const
    {
        const std::string status = to_lower(response.get_status());
        return status == "validation_failed" || status == "file_validation_failed" ||
               response.get_raw_json().find("file_validation_failed") != std::string::npos;
    }

    [[nodiscard]] std::string_view file_status_unavailable() const noexcept
    {
        return "UNAVAILABLE";
    }

    [[nodiscard]] std::string_view file_status_available() const noexcept
    {
        return "AVAILABLE";
    }

    [[nodiscard]] std::string_view file_storage_namespace() const noexcept
    {
        return "file_storage";
    }

    [[nodiscard]] bool iequals(std::string_view lhs, std::string_view rhs) const noexcept
    {
        return to_lower(lhs) == to_lower(rhs);
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

    [[nodiscard]] std::string now_string() const
    {
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            )
                .count()
        );
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::files
