export module app_handlers_files:runtime_retrieve;

import std;
import core_error;
import app_context;
import app_metrics;
import app_store;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::files {

class FilesRetrieveRuntime
{
public:
    explicit FilesRetrieveRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief Port of `files_retrieve_core` (`core/files.rs:111-146`). */
    [[nodiscard]] std::expected<FilesRetrieveResult, core::error::AppError>
    retrieve(const FilesRetrieveInput& input) const
    {
        // Upstream: files.rs:151-152 — server_wrap records the flow and route counter.
        m_context.metrics().record_flow("files.retrieve");
        m_context.metrics().counter("files_retrieve_total").increment();

        // core/files.rs:116-125 — find_file_metadata... . FileNotFound when absent.
        const auto metadata = m_support.find_file_metadata_by_processor_merchant_id_file_id(
            input.processor_merchant_id,
            input.file_id
        );
        if (!metadata) {
            return std::unexpected{
                Builder{}(Kind::NOT_FOUND, "File does not exist in our records")
            };
        }

        // core/files.rs:126-133 — FileDataRequired::Required.
        auto info = m_support.retrieve_file_and_provider_file_id_from_file_id(
            input.processor_merchant_id,
            input.file_id,
            input.dispute_id,
            FileDataRequired::Required
        );
        if (!info) {
            return std::unexpected{std::move(info.error())};
        }

        // core/files.rs:134-138 — file_type.parse::<mime::Mime>().
        if (!m_support.is_valid_mime(metadata->get_file_type())) {
            return std::unexpected{
                Builder{}(Kind::INTERNAL_ERROR, "Failed to parse file content type")
            };
        }
        // core/files.rs:139-145 — file_data.ok_or(FileNotAvailable).
        if (!info->file_data.has_value()) {
            return std::unexpected{Builder{}(Kind::NOT_FOUND, "File not available")};
        }

        FilesRetrieveResult result{};
        result.set_file_data(*info->file_data);
        result.set_content_type(metadata->get_file_type());
        return result;
    }

private:
    app::context::AppContext& m_context;
    FilesRuntimeSupport m_support;
};

} // namespace app::handlers::files
