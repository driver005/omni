module;
export module app_dto:hierarchical_resources;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::hierarchical_resources {

class PrimitiveDateTimeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ResourceIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ResourceRequestorTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ResourceTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class UploadCertificateRequest
{
public:
    void set_certificate(std::string value) { certificate = std::move(value); }
    [[nodiscard]] const std::string& get_certificate() const noexcept { return certificate; }
    std::string certificate{};
};

class GenerateHierarchicalResourceRequest
{
public:
    void set_resource_type(ResourceTypeValue value) { resource_type = std::move(value); }
    [[nodiscard]] const ResourceTypeValue& get_resource_type() const noexcept { return resource_type; }
    ResourceTypeValue resource_type{};
};

class GenerateHierarchicalResourceResponse
{
public:
    void set_id(ResourceIdValue value) { id = std::move(value); }
    [[nodiscard]] const ResourceIdValue& get_id() const noexcept { return id; }
    void set_data(std::string value) { data = std::move(value); }
    [[nodiscard]] const std::string& get_data() const noexcept { return data; }
    ResourceIdValue id{};
    std::string data{};
};

class HierarchicalResourceSummary
{
public:
    void set_id(ResourceIdValue value) { id = std::move(value); }
    [[nodiscard]] const ResourceIdValue& get_id() const noexcept { return id; }
    void set_display_schema(std::string value) { display_schema = std::move(value); }
    [[nodiscard]] const std::string& get_display_schema() const noexcept { return display_schema; }
    void set_display_data(std::string value) { display_data = std::move(value); }
    [[nodiscard]] const std::string& get_display_data() const noexcept { return display_data; }
    void set_created_at(PrimitiveDateTimeValue value) { created_at = std::move(value); }
    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept { return created_at; }
    void set_is_linked(bool value) { is_linked = std::move(value); }
    [[nodiscard]] const bool& get_is_linked() const noexcept { return is_linked; }
    ResourceIdValue id{};
    std::string display_schema{};
    std::string display_data{};
    PrimitiveDateTimeValue created_at{};
    bool is_linked{};
};

class LinkHierarchicalResourceRequest
{
public:
    void set_requestor_type(ResourceRequestorTypeValue value) { requestor_type = std::move(value); }
    [[nodiscard]] const ResourceRequestorTypeValue& get_requestor_type() const noexcept { return requestor_type; }
    void set_requestor_id(std::string value) { requestor_id = std::move(value); }
    [[nodiscard]] const std::string& get_requestor_id() const noexcept { return requestor_id; }
    ResourceRequestorTypeValue requestor_type{};
    std::string requestor_id{};
};

class LinkHierarchicalResourceResponse
{
public:
    void set_id(ResourceIdValue value) { id = std::move(value); }
    [[nodiscard]] const ResourceIdValue& get_id() const noexcept { return id; }
    ResourceIdValue id{};
};

class ListHierarchicalResourcesRequest
{
public:
    void set_resource_type(ResourceTypeValue value) { resource_type = std::move(value); }
    [[nodiscard]] const ResourceTypeValue& get_resource_type() const noexcept { return resource_type; }
    void set_scope_id(std::optional<std::string> value) { scope_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_scope_id() const noexcept { return scope_id; }
    void set_scope_type(std::optional<ResourceRequestorTypeValue> value) { scope_type = std::move(value); }
    [[nodiscard]] const std::optional<ResourceRequestorTypeValue>& get_scope_type() const noexcept { return scope_type; }
    ResourceTypeValue resource_type{};
    std::optional<std::string> scope_id{};
    std::optional<ResourceRequestorTypeValue> scope_type{};
};

class UploadCertificateResponse
{
public:
    void set_id(ResourceIdValue value) { id = std::move(value); }
    [[nodiscard]] const ResourceIdValue& get_id() const noexcept { return id; }
    void set_created_at(PrimitiveDateTimeValue value) { created_at = std::move(value); }
    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept { return created_at; }
    ResourceIdValue id{};
    PrimitiveDateTimeValue created_at{};
};

class ListHierarchicalResourcesResponse
{
public:
    void set_resources(std::vector<HierarchicalResourceSummary> value) { resources = std::move(value); }
    [[nodiscard]] const std::vector<HierarchicalResourceSummary>& get_resources() const noexcept { return resources; }
    std::vector<HierarchicalResourceSummary> resources{};
};
} // namespace app::dto::hierarchical_resources

template<>
struct serde::Serializable<app::dto::hierarchical_resources::GenerateHierarchicalResourceRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"type", &app::dto::hierarchical_resources::GenerateHierarchicalResourceRequest::get_resource_type, &app::dto::hierarchical_resources::GenerateHierarchicalResourceRequest::set_resource_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "generate_hierarchical_resource_request"; }
};

template<>
struct serde::Serializable<app::dto::hierarchical_resources::GenerateHierarchicalResourceResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::dto::hierarchical_resources::GenerateHierarchicalResourceResponse::get_id, &app::dto::hierarchical_resources::GenerateHierarchicalResourceResponse::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"data", &app::dto::hierarchical_resources::GenerateHierarchicalResourceResponse::get_data, &app::dto::hierarchical_resources::GenerateHierarchicalResourceResponse::set_data, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "generate_hierarchical_resource_response"; }
};

template<>
struct serde::Serializable<app::dto::hierarchical_resources::UploadCertificateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"certificate", &app::dto::hierarchical_resources::UploadCertificateRequest::get_certificate, &app::dto::hierarchical_resources::UploadCertificateRequest::set_certificate, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "upload_certificate_request"; }
};

template<>
struct serde::Serializable<app::dto::hierarchical_resources::UploadCertificateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::dto::hierarchical_resources::UploadCertificateResponse::get_id, &app::dto::hierarchical_resources::UploadCertificateResponse::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::dto::hierarchical_resources::UploadCertificateResponse::get_created_at, &app::dto::hierarchical_resources::UploadCertificateResponse::set_created_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "upload_certificate_response"; }
};

template<>
struct serde::Serializable<app::dto::hierarchical_resources::HierarchicalResourceSummary>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::dto::hierarchical_resources::HierarchicalResourceSummary::get_id, &app::dto::hierarchical_resources::HierarchicalResourceSummary::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"display_schema", &app::dto::hierarchical_resources::HierarchicalResourceSummary::get_display_schema, &app::dto::hierarchical_resources::HierarchicalResourceSummary::set_display_schema, serde::FieldOptions{}>{},
            serde::FieldDesc<"display_data", &app::dto::hierarchical_resources::HierarchicalResourceSummary::get_display_data, &app::dto::hierarchical_resources::HierarchicalResourceSummary::set_display_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::dto::hierarchical_resources::HierarchicalResourceSummary::get_created_at, &app::dto::hierarchical_resources::HierarchicalResourceSummary::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_linked", &app::dto::hierarchical_resources::HierarchicalResourceSummary::get_is_linked, &app::dto::hierarchical_resources::HierarchicalResourceSummary::set_is_linked, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "hierarchical_resource_summary"; }
};

template<>
struct serde::Serializable<app::dto::hierarchical_resources::ListHierarchicalResourcesRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"type", &app::dto::hierarchical_resources::ListHierarchicalResourcesRequest::get_resource_type, &app::dto::hierarchical_resources::ListHierarchicalResourcesRequest::set_resource_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"scope_id", &app::dto::hierarchical_resources::ListHierarchicalResourcesRequest::get_scope_id, &app::dto::hierarchical_resources::ListHierarchicalResourcesRequest::set_scope_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"scope_type", &app::dto::hierarchical_resources::ListHierarchicalResourcesRequest::get_scope_type, &app::dto::hierarchical_resources::ListHierarchicalResourcesRequest::set_scope_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "list_hierarchical_resources_request"; }
};

template<>
struct serde::Serializable<app::dto::hierarchical_resources::ListHierarchicalResourcesResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"resources", &app::dto::hierarchical_resources::ListHierarchicalResourcesResponse::get_resources, &app::dto::hierarchical_resources::ListHierarchicalResourcesResponse::set_resources, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "list_hierarchical_resources_response"; }
};

template<>
struct serde::Serializable<app::dto::hierarchical_resources::LinkHierarchicalResourceRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"requestor_type", &app::dto::hierarchical_resources::LinkHierarchicalResourceRequest::get_requestor_type, &app::dto::hierarchical_resources::LinkHierarchicalResourceRequest::set_requestor_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"requestor_id", &app::dto::hierarchical_resources::LinkHierarchicalResourceRequest::get_requestor_id, &app::dto::hierarchical_resources::LinkHierarchicalResourceRequest::set_requestor_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "link_hierarchical_resource_request"; }
};

template<>
struct serde::Serializable<app::dto::hierarchical_resources::LinkHierarchicalResourceResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::dto::hierarchical_resources::LinkHierarchicalResourceResponse::get_id, &app::dto::hierarchical_resources::LinkHierarchicalResourceResponse::set_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "link_hierarchical_resource_response"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::hierarchical_resources::tests {
using namespace boost::ut;
suite<<app_dto_hierarchical_resources>> app_dto_hierarchical_resources_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::hierarchical_resources::GenerateHierarchicalResourceRequest>::fields())> > 0);
    };
};
}
#endif
