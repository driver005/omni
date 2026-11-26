export module app_user:hierarchical_resources_result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::hierarchical_resources {

// The typed response DTOs are the exact shapes the old handler encoded with
// serde::Ser::encode_json. Deriving from them and reusing their Serializable
// field descriptors preserves field names, order and RawJson embedding.

class GenerateHierarchicalResourceResult :
    public ::app::dto::hierarchical_resources::GenerateHierarchicalResourceResponse
{
public:
    GenerateHierarchicalResourceResult() = default;

    explicit GenerateHierarchicalResourceResult(
        ::app::dto::hierarchical_resources::GenerateHierarchicalResourceResponse value
    ) noexcept :
        GenerateHierarchicalResourceResponse{std::move(value)}
    {
    }
};

class UploadHierarchicalResourceResult :
    public ::app::dto::hierarchical_resources::UploadCertificateResponse
{
public:
    UploadHierarchicalResourceResult() = default;

    explicit UploadHierarchicalResourceResult(
        ::app::dto::hierarchical_resources::UploadCertificateResponse value
    ) noexcept :
        UploadCertificateResponse{std::move(value)}
    {
    }
};

class ListHierarchicalResourcesResult :
    public ::app::dto::hierarchical_resources::ListHierarchicalResourcesResponse
{
public:
    ListHierarchicalResourcesResult() = default;

    explicit ListHierarchicalResourcesResult(
        ::app::dto::hierarchical_resources::ListHierarchicalResourcesResponse value
    ) noexcept :
        ListHierarchicalResourcesResponse{std::move(value)}
    {
    }
};

class LinkHierarchicalResourceResult :
    public ::app::dto::hierarchical_resources::LinkHierarchicalResourceResponse
{
public:
    LinkHierarchicalResourceResult() = default;

    explicit LinkHierarchicalResourceResult(
        ::app::dto::hierarchical_resources::LinkHierarchicalResourceResponse value
    ) noexcept :
        LinkHierarchicalResourceResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::hierarchical_resources

template<>
struct serde::Serializable<
    ::app::handlers::hierarchical_resources::GenerateHierarchicalResourceResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            ::app::dto::hierarchical_resources::GenerateHierarchicalResourceResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "generate_hierarchical_resource_result";
    }
};

template<>
struct serde::Serializable<
    ::app::handlers::hierarchical_resources::UploadHierarchicalResourceResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            ::app::dto::hierarchical_resources::UploadCertificateResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "upload_hierarchical_resource_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::hierarchical_resources::ListHierarchicalResourcesResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            ::app::dto::hierarchical_resources::ListHierarchicalResourcesResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "list_hierarchical_resources_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::hierarchical_resources::LinkHierarchicalResourceResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            ::app::dto::hierarchical_resources::LinkHierarchicalResourceResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "link_hierarchical_resource_result";
    }
};
