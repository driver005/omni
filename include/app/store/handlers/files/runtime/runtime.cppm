export module app_store:files_runtime;

import std;
import core_error;
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

    [[nodiscard]] std::expected<FilesCreateResult, core::error::AppError>
    create(const FilesCreateInput& input) const
    {
        return m_create.create(input);
    }

    [[nodiscard]] std::expected<FilesDeleteResult, core::error::AppError>
    delete_file(const FilesDeleteInput& input) const
    {
        return m_delete.delete_file(input);
    }

    [[nodiscard]] std::expected<FilesRetrieveResult, core::error::AppError>
    retrieve(const FilesRetrieveInput& input) const
    {
        return m_retrieve.retrieve(input);
    }

private:
    ::app::context::AppContext& m_context;
    FilesCreateRuntime m_create;
    FilesDeleteRuntime m_delete;
    FilesRetrieveRuntime m_retrieve;
};

} // namespace app::handlers::files
