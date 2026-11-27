export module app_store:files_runtime;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_dto;
import app_context;

import :files_input;
import :files_result;
import :files_runtime_common;
import :files_runtime_create;
import :files_runtime_delete;
import :files_runtime_retrieve;

export namespace app::handlers::files {

class FilesRuntime
{
public:
    explicit FilesRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_delete{context},
        m_retrieve{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::files::CreateFileResponse>
    create(
        const CreateFileRequestData& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_create.create(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    delete_file(const core::behavior::CallContext& ctx) const
    {
        return m_delete.delete_file(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    retrieve(const core::behavior::CallContext& ctx) const
    {
        return m_retrieve.retrieve(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    FilesCreateRuntime m_create;
    FilesDeleteRuntime m_delete;
    FilesRetrieveRuntime m_retrieve;
};

} // namespace app::handlers::files
