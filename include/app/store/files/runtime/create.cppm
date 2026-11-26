export module app_store:files_runtime_create;

import std;
import core_error;
import app_context;
import app_dto;
import app_metrics;
import app_context_core;

import :files_input;
import :files_result;
import :files_runtime_common;

export namespace app::handlers::files {

class FilesCreateRuntime
{
public:
    explicit FilesCreateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /**
     * @brief Port of `files_create_core` (`core/files.rs:14-90`).
     *  validate -> generate_id -> insert(available=false) -> upload -> update(available=true).
     */
    [[nodiscard]] std::expected<FilesCreateResult, core::error::AppError>
    create(const FilesCreateInput& input) const
    {
        // Upstream: files.rs:39 — server_wrap records the flow and route counter.
        m_context.metrics().record_flow("files.create");
        m_context.metrics().counter("files_create_total").increment();

        CreateFileRequestData request{};
        request.file = input.request.file;
        request.file_name = input.request.file_name;
        request.file_size = input.request.file_size;
        request.file_type = input.request.file_type;
        request.purpose = input.request.purpose;
        request.dispute_id = input.request.dispute_id;

        // core/files.rs:19-24.
        if (const auto invalid =
                m_support.validate_file_upload(request, input.processor_merchant_id)) {
            return std::unexpected{*invalid};
        }

        // core/files.rs:25 — common_utils::generate_id(consts::ID_LENGTH=20, "file").
        const auto file_id = m_support.generate_id();
        // core/files.rs:26-34 — file_key = "{processor_merchant_id}/{file_id}".
        const std::string file_key = std::format("{}/{}", input.processor_merchant_id, file_id);

        // core/files.rs:35-59 — FileMetadataNew { available: false, provider_file_id: None,
        // file_upload_provider: None, profile_id: None, merchant_connector_id: None }.
        FileRecord record{};
        record.set_file_id(file_id);
        record.set_merchant_id(std::string{input.provider_merchant_id});
        record.set_profile_id(std::string{});
        record.set_file_name(input.request.file_name.value_or(std::string{}));
        record.set_file_type(input.request.file_type);
        record.set_file_size(static_cast<std::int64_t>(input.request.file_size));
        record.set_purpose(std::string{m_support.file_purpose_name(input.request.purpose)});
        record.set_provider_file_id(std::string{});
        record.set_upload_provider(std::string{});
        record.set_status(std::string{m_support.file_status_unavailable()});
        record.set_created_at(m_support.now_string());
        auto inserted = m_support.insert_file_metadata(record);
        if (!inserted) {
            return std::unexpected{std::move(inserted.error())};
        }
        record = *inserted;

        // core/files.rs:60-68.
        auto uploaded = m_support.upload_and_get_provider_provider_file_id_profile_id(
            request,
            file_key,
            input.processor_merchant_id
        );
        if (!uploaded) {
            return std::unexpected{std::move(uploaded.error())};
        }

        // core/files.rs:70-86 — FileMetadataUpdate::Update { provider_file_id,
        // file_upload_provider, available: true, profile_id, merchant_connector_id }.
        record.set_provider_file_id(uploaded->provider_file_id);
        record.set_upload_provider(
            std::string{m_support.file_upload_provider_name(uploaded->provider)}
        );
        record.set_status(std::string{m_support.file_status_available()});
        if (uploaded->profile_id.has_value()) {
            record.set_profile_id(*uploaded->profile_id);
        }
        if (!m_support.update_file_metadata(record)) {
            return std::unexpected{Builder{}(
                Kind::INTERNAL_ERROR,
                std::format("Unable to update file_metadata with file_id: {}", file_id)
            )};
        }

        // core/files.rs:87-89 — Json(CreateFileResponse { file_id }).
        ::app::dto::files::CreateFileResponse response{};
        response.set_file_id(file_id);
        return FilesCreateResult{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    FilesRuntimeSupport m_support;
};

} // namespace app::handlers::files
