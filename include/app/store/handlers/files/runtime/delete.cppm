export module app_store:files_runtime_delete;

import std;
import core_error;
import app_context;
import app_metrics;
import app_context_core;

import :files_input;
import :files_result;
import :files_runtime_common;

export namespace app::handlers::files {

class FilesDeleteRuntime
{
public:
    explicit FilesDeleteRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief Port of `files_delete_core` (`core/files.rs:92-109`). */
    [[nodiscard]] std::expected<FilesDeleteResult, core::error::AppError>
    delete_file(const FilesDeleteInput& input) const
    {
        // Upstream: files.rs:96-97 — server_wrap records the flow and route counter.
        m_context.metrics().record_flow("files.delete");
        m_context.metrics().counter("files_delete_total").increment();

        // core/files.rs:97 — delete_file_using_file_id (guards + file-storage delete).
        const auto record =
            m_support.delete_file_using_file_id(input.processor_merchant_id, input.file_id);
        if (!record) {
            return std::unexpected{std::move(record.error())};
        }
        // core/files.rs:98-107 — delete_file_metadata_by_processor_merchant_id_file_id.
        if (!m_context.repository().remove_file(record->get_id())) {
            return std::unexpected{
                Builder{}(Kind::INTERNAL_ERROR, "Unable to delete file_metadata")
            };
        }

        // core/files.rs:108-109 — Json(ApplicationResponse::StatusOk { "success" }).
        FilesDeleteResult result{};
        result.set_status("success");
        return result;
    }

private:
    ::app::context::AppContext& m_context;
    FilesRuntimeSupport m_support;
};

} // namespace app::handlers::files
