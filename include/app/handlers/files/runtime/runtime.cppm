export module app_handlers_files:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_create;
import :runtime_delete;
import :runtime_retrieve;

export namespace app::handlers::files {

class FilesRuntime
{
public:
    explicit FilesRuntime(app::context::AppContext& context) noexcept :
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
    app::context::AppContext& m_context;
    FilesCreateRuntime m_create;
    FilesDeleteRuntime m_delete;
    FilesRetrieveRuntime m_retrieve;
};

} // namespace app::handlers::files
