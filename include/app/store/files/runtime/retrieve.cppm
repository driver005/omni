export module app_store:files_runtime_retrieve;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_context;
import app_metrics;
import app_context_core;

import :files_input;
import :files_result;
import :files_runtime_common;

export namespace app::handlers::files {

class FilesRetrieveRuntime
{
public:
    explicit FilesRetrieveRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief Port of `files_retrieve_core` (`core/files.rs:111-146`). */
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    retrieve(const core::behavior::CallContext& ctx) const
    {
        // Upstream: files.rs:151-152 — server_wrap records the flow and route counter.
        m_context.logger().record_flow("files.retrieve");
        m_context.logger().counter("files_retrieve_total").increment();

        const auto file_id = m_support.file_id_from(ctx);
        if (!file_id.has_value()) {
            return std::unexpected{Builder{}(Kind::INVALID_REQUEST, "missing file id in path")};
        }
        std::optional<std::string> dispute_id{};
        if (const auto it = ctx.query.find("dispute_id"); it != ctx.query.end()) {
            dispute_id = it->second;
        }

        // core/files.rs:116-125 — find_file_metadata... . FileNotFound when absent.
        const auto metadata = m_support.find_file_metadata_by_processor_merchant_id_file_id(
            ctx.merchant_id,
            *file_id
        );
        if (!metadata) {
            return std::unexpected{
                Builder{}(Kind::NOT_FOUND, "File does not exist in our records")
            };
        }

        // core/files.rs:126-133 — FileDataRequired::Required.
        auto info = m_support.retrieve_file_and_provider_file_id_from_file_id(
            ctx.merchant_id,
            *file_id,
            dispute_id,
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

        serde::Value::Object document{};
        document.insert(
            std::string{files_retrieve_wire_fields()[0]},
            serde::Value{*info->file_data}
        );
        document.insert(
            std::string{files_retrieve_wire_fields()[1]},
            serde::Value{metadata->get_file_type()}
        );
        return serde::Value{std::move(document)};
    }

private:
    ::app::context::AppContext& m_context;
    FilesRuntimeSupport m_support;
};

} // namespace app::handlers::files
