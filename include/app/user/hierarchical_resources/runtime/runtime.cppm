export module app_user:hierarchical_resources_runtime;

import std;
import core_error;
import app_context;

import :hierarchical_resources_input;
import :hierarchical_resources_result;
import :hierarchical_resources_runtime_common;
import :hierarchical_resources_runtime_generate;
import :hierarchical_resources_runtime_upload;
import :hierarchical_resources_runtime_list;
import :hierarchical_resources_runtime_link;

export namespace app::handlers::hierarchical_resources {

class HierarchicalResourcesRuntime
{
public:
    explicit HierarchicalResourcesRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_generate{context},
        m_upload{context},
        m_list{context},
        m_link{context}
    {
    }

    [[nodiscard]] std::expected<GenerateHierarchicalResourceResult, core::error::AppError>
    generate(const GenerateHierarchicalResourceInput& input) const
    {
        return m_generate.generate(input);
    }

    [[nodiscard]] std::expected<UploadHierarchicalResourceResult, core::error::AppError>
    upload(const UploadHierarchicalResourceInput& input) const
    {
        return m_upload.upload(input);
    }

    [[nodiscard]] std::expected<ListHierarchicalResourcesResult, core::error::AppError>
    list(const ListHierarchicalResourcesInput& input) const
    {
        return m_list.list(input);
    }

    [[nodiscard]] std::expected<LinkHierarchicalResourceResult, core::error::AppError>
    link(const LinkHierarchicalResourceInput& input) const
    {
        return m_link.link(input);
    }

private:
    ::app::context::AppContext& m_context;
    GenerateHierarchicalResourceRuntime m_generate;
    UploadHierarchicalResourceRuntime m_upload;
    ListHierarchicalResourcesRuntime m_list;
    LinkHierarchicalResourceRuntime m_link;
};

} // namespace app::handlers::hierarchical_resources
