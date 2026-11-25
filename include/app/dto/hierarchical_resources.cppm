module;
#include <simdjson.h>
#include <toml++/toml.hpp>
export module app_dto:hierarchical_resources;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::hierarchical_resources {

// ── Wire value objects ───────────────────────────────────────────────────────
// Upstream `id_type::ResourceId`, `time::PrimitiveDateTime` and the two
// `common_enums` newtypes all serialize as plain JSON strings (see the
// `FieldConverter` specializations below). These typed wrappers keep the
// compile-time distinction while emitting the exact upstream wire shape.

class PrimitiveDateTimeValue
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ResourceIdValue
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ResourceRequestorTypeValue
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ResourceTypeValue
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class UploadCertificateRequest
{
public:
    void set_certificate(std::string value)
    {
        certificate = std::move(value);
    }

    [[nodiscard]] const std::string& get_certificate() const noexcept
    {
        return certificate;
    }

    std::string certificate{};
};

class GenerateHierarchicalResourceRequest
{
public:
    void set_resource_type(ResourceTypeValue value)
    {
        resource_type = std::move(value);
    }

    [[nodiscard]] const ResourceTypeValue& get_resource_type() const noexcept
    {
        return resource_type;
    }

    ResourceTypeValue resource_type{};
};

// ── Real typed response DTOs (api_models/src/hierarchical_resources.rs) ──────
// `data`/`display_schema`/`display_data` mirror upstream `Secret<serde_json::Value>`
// fields: they are carried as pre-rendered JSON fragments (serde::RawJson) so they
// are emitted as embedded JSON objects, never as escaped strings.

class GenerateHierarchicalResourceResponse
{
public:
    void set_id(ResourceIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const ResourceIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_data(serde::RawJson value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const serde::RawJson& get_data() const noexcept
    {
        return data;
    }

    ResourceIdValue id{};
    serde::RawJson data{};
};

class HierarchicalResourceSummary
{
public:
    void set_id(ResourceIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const ResourceIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_display_schema(serde::RawJson value)
    {
        display_schema = std::move(value);
    }

    [[nodiscard]] const serde::RawJson& get_display_schema() const noexcept
    {
        return display_schema;
    }

    void set_display_data(serde::RawJson value)
    {
        display_data = std::move(value);
    }

    [[nodiscard]] const serde::RawJson& get_display_data() const noexcept
    {
        return display_data;
    }

    void set_created_at(PrimitiveDateTimeValue value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_is_linked(bool value)
    {
        is_linked = std::move(value);
    }

    [[nodiscard]] const bool& get_is_linked() const noexcept
    {
        return is_linked;
    }

    ResourceIdValue id{};
    serde::RawJson display_schema{};
    serde::RawJson display_data{};
    PrimitiveDateTimeValue created_at{};
    bool is_linked{};
};

class LinkHierarchicalResourceRequest
{
public:
    void set_requestor_type(ResourceRequestorTypeValue value)
    {
        requestor_type = std::move(value);
    }

    [[nodiscard]] const ResourceRequestorTypeValue& get_requestor_type() const noexcept
    {
        return requestor_type;
    }

    void set_requestor_id(std::string value)
    {
        requestor_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_requestor_id() const noexcept
    {
        return requestor_id;
    }

    ResourceRequestorTypeValue requestor_type{};
    std::string requestor_id{};
};

class LinkHierarchicalResourceResponse
{
public:
    void set_id(ResourceIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const ResourceIdValue& get_id() const noexcept
    {
        return id;
    }

    ResourceIdValue id{};
};

class ListHierarchicalResourcesRequest
{
public:
    void set_resource_type(ResourceTypeValue value)
    {
        resource_type = std::move(value);
    }

    [[nodiscard]] const ResourceTypeValue& get_resource_type() const noexcept
    {
        return resource_type;
    }

    void set_scope_id(std::optional<std::string> value)
    {
        scope_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_scope_id() const noexcept
    {
        return scope_id;
    }

    void set_scope_type(std::optional<ResourceRequestorTypeValue> value)
    {
        scope_type = std::move(value);
    }

    [[nodiscard]] const std::optional<ResourceRequestorTypeValue>& get_scope_type() const noexcept
    {
        return scope_type;
    }

    ResourceTypeValue resource_type{};
    std::optional<std::string> scope_id{};
    std::optional<ResourceRequestorTypeValue> scope_type{};
};

class UploadCertificateResponse
{
public:
    void set_id(ResourceIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const ResourceIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_created_at(PrimitiveDateTimeValue value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept
    {
        return created_at;
    }

    ResourceIdValue id{};
    PrimitiveDateTimeValue created_at{};
};

class ListHierarchicalResourcesResponse
{
public:
    void set_resources(std::vector<HierarchicalResourceSummary> value)
    {
        resources = std::move(value);
    }

    [[nodiscard]] const std::vector<HierarchicalResourceSummary>& get_resources() const noexcept
    {
        return resources;
    }

    std::vector<HierarchicalResourceSummary> resources{};
};
} // namespace app::dto::hierarchical_resources

// ── Plain-string wire converters for the value newtypes ──────────────────────
// These explicit full specializations override the generic `ISerializable`
// `FieldConverter` so ids/dates/enums ride the wire as plain strings, exactly
// like upstream. The pattern mirrors app/dto/api_keys.cppm.

template<>
struct serde::FieldConverter<app::dto::hierarchical_resources::ResourceIdValue>
{
    using rfl_type = std::string;

    static rfl_type to_rfl(const app::dto::hierarchical_resources::ResourceIdValue& v)
    {
        return v.get_value();
    }

    static app::dto::hierarchical_resources::ResourceIdValue from_rfl(const rfl_type& v)
    {
        app::dto::hierarchical_resources::ResourceIdValue out;
        out.set_value(v);
        return out;
    }

    static simdjson::error_code from_simdjson(
        simdjson::ondemand::value& j,
        app::dto::hierarchical_resources::ResourceIdValue& out
    )
    {
        std::string_view s;
        if (auto ec = j.get_string().get(s); ec) {
            return ec;
        }
        out.set_value(std::string{s});
        return simdjson::SUCCESS;
    }

    static std::expected<app::dto::hierarchical_resources::ResourceIdValue, std::string>
    from_toml(const toml::table& table, std::string_view name)
    {
        auto v = table[name].value<std::string>();
        if (!v) {
            return std::unexpected{std::format("missing field '{}'", name)};
        }
        app::dto::hierarchical_resources::ResourceIdValue out;
        out.set_value(*v);
        return out;
    }
};

template<>
struct serde::FieldConverter<app::dto::hierarchical_resources::PrimitiveDateTimeValue>
{
    using rfl_type = std::string;

    static rfl_type to_rfl(const app::dto::hierarchical_resources::PrimitiveDateTimeValue& v)
    {
        return v.get_value();
    }

    static app::dto::hierarchical_resources::PrimitiveDateTimeValue from_rfl(const rfl_type& v)
    {
        app::dto::hierarchical_resources::PrimitiveDateTimeValue out;
        out.set_value(v);
        return out;
    }

    static simdjson::error_code from_simdjson(
        simdjson::ondemand::value& j,
        app::dto::hierarchical_resources::PrimitiveDateTimeValue& out
    )
    {
        std::string_view s;
        if (auto ec = j.get_string().get(s); ec) {
            return ec;
        }
        out.set_value(std::string{s});
        return simdjson::SUCCESS;
    }

    static std::expected<app::dto::hierarchical_resources::PrimitiveDateTimeValue, std::string>
    from_toml(const toml::table& table, std::string_view name)
    {
        auto v = table[name].value<std::string>();
        if (!v) {
            return std::unexpected{std::format("missing field '{}'", name)};
        }
        app::dto::hierarchical_resources::PrimitiveDateTimeValue out;
        out.set_value(*v);
        return out;
    }
};

template<>
struct serde::FieldConverter<app::dto::hierarchical_resources::ResourceTypeValue>
{
    using rfl_type = std::string;

    static rfl_type to_rfl(const app::dto::hierarchical_resources::ResourceTypeValue& v)
    {
        return v.get_value();
    }

    static app::dto::hierarchical_resources::ResourceTypeValue from_rfl(const rfl_type& v)
    {
        app::dto::hierarchical_resources::ResourceTypeValue out;
        out.set_value(v);
        return out;
    }

    static simdjson::error_code from_simdjson(
        simdjson::ondemand::value& j,
        app::dto::hierarchical_resources::ResourceTypeValue& out
    )
    {
        std::string_view s;
        if (auto ec = j.get_string().get(s); ec) {
            return ec;
        }
        out.set_value(std::string{s});
        return simdjson::SUCCESS;
    }

    static std::expected<app::dto::hierarchical_resources::ResourceTypeValue, std::string>
    from_toml(const toml::table& table, std::string_view name)
    {
        auto v = table[name].value<std::string>();
        if (!v) {
            return std::unexpected{std::format("missing field '{}'", name)};
        }
        app::dto::hierarchical_resources::ResourceTypeValue out;
        out.set_value(*v);
        return out;
    }
};

template<>
struct serde::FieldConverter<app::dto::hierarchical_resources::ResourceRequestorTypeValue>
{
    using rfl_type = std::string;

    static rfl_type to_rfl(const app::dto::hierarchical_resources::ResourceRequestorTypeValue& v)
    {
        return v.get_value();
    }

    static app::dto::hierarchical_resources::ResourceRequestorTypeValue from_rfl(const rfl_type& v)
    {
        app::dto::hierarchical_resources::ResourceRequestorTypeValue out;
        out.set_value(v);
        return out;
    }

    static simdjson::error_code from_simdjson(
        simdjson::ondemand::value& j,
        app::dto::hierarchical_resources::ResourceRequestorTypeValue& out
    )
    {
        std::string_view s;
        if (auto ec = j.get_string().get(s); ec) {
            return ec;
        }
        out.set_value(std::string{s});
        return simdjson::SUCCESS;
    }

    static std::expected<app::dto::hierarchical_resources::ResourceRequestorTypeValue, std::string>
    from_toml(const toml::table& table, std::string_view name)
    {
        auto v = table[name].value<std::string>();
        if (!v) {
            return std::unexpected{std::format("missing field '{}'", name)};
        }
        app::dto::hierarchical_resources::ResourceRequestorTypeValue out;
        out.set_value(*v);
        return out;
    }
};

template<>
struct serde::Serializable<app::dto::hierarchical_resources::GenerateHierarchicalResourceRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "type",
            &app::dto::hierarchical_resources::GenerateHierarchicalResourceRequest::
                get_resource_type,
            &app::dto::hierarchical_resources::GenerateHierarchicalResourceRequest::
                set_resource_type,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "generate_hierarchical_resource_request";
    }
};

template<>
struct serde::Serializable<app::dto::hierarchical_resources::GenerateHierarchicalResourceResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::dto::hierarchical_resources::GenerateHierarchicalResourceResponse::get_id,
                &app::dto::hierarchical_resources::GenerateHierarchicalResourceResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::dto::hierarchical_resources::GenerateHierarchicalResourceResponse::get_data,
                &app::dto::hierarchical_resources::GenerateHierarchicalResourceResponse::set_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "generate_hierarchical_resource_response";
    }
};

template<>
struct serde::Serializable<app::dto::hierarchical_resources::UploadCertificateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "certificate",
            &app::dto::hierarchical_resources::UploadCertificateRequest::get_certificate,
            &app::dto::hierarchical_resources::UploadCertificateRequest::set_certificate,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "upload_certificate_request";
    }
};

template<>
struct serde::Serializable<app::dto::hierarchical_resources::UploadCertificateResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::dto::hierarchical_resources::UploadCertificateResponse::get_id,
                &app::dto::hierarchical_resources::UploadCertificateResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::dto::hierarchical_resources::UploadCertificateResponse::get_created_at,
                &app::dto::hierarchical_resources::UploadCertificateResponse::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "upload_certificate_response";
    }
};

template<>
struct serde::Serializable<app::dto::hierarchical_resources::HierarchicalResourceSummary>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::dto::hierarchical_resources::HierarchicalResourceSummary::get_id,
                &app::dto::hierarchical_resources::HierarchicalResourceSummary::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "display_schema",
                &app::dto::hierarchical_resources::HierarchicalResourceSummary::get_display_schema,
                &app::dto::hierarchical_resources::HierarchicalResourceSummary::set_display_schema,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "display_data",
                &app::dto::hierarchical_resources::HierarchicalResourceSummary::get_display_data,
                &app::dto::hierarchical_resources::HierarchicalResourceSummary::set_display_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::dto::hierarchical_resources::HierarchicalResourceSummary::get_created_at,
                &app::dto::hierarchical_resources::HierarchicalResourceSummary::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_linked",
                &app::dto::hierarchical_resources::HierarchicalResourceSummary::get_is_linked,
                &app::dto::hierarchical_resources::HierarchicalResourceSummary::set_is_linked,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "hierarchical_resource_summary";
    }
};

template<>
struct serde::Serializable<app::dto::hierarchical_resources::ListHierarchicalResourcesRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "type",
                &app::dto::hierarchical_resources::ListHierarchicalResourcesRequest::
                    get_resource_type,
                &app::dto::hierarchical_resources::ListHierarchicalResourcesRequest::
                    set_resource_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "scope_id",
                &app::dto::hierarchical_resources::ListHierarchicalResourcesRequest::get_scope_id,
                &app::dto::hierarchical_resources::ListHierarchicalResourcesRequest::set_scope_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "scope_type",
                &app::dto::hierarchical_resources::ListHierarchicalResourcesRequest::get_scope_type,
                &app::dto::hierarchical_resources::ListHierarchicalResourcesRequest::set_scope_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "list_hierarchical_resources_request";
    }
};

template<>
struct serde::Serializable<app::dto::hierarchical_resources::ListHierarchicalResourcesResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "resources",
            &app::dto::hierarchical_resources::ListHierarchicalResourcesResponse::get_resources,
            &app::dto::hierarchical_resources::ListHierarchicalResourcesResponse::set_resources,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "list_hierarchical_resources_response";
    }
};

template<>
struct serde::Serializable<app::dto::hierarchical_resources::LinkHierarchicalResourceRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "requestor_type",
                &app::dto::hierarchical_resources::LinkHierarchicalResourceRequest::
                    get_requestor_type,
                &app::dto::hierarchical_resources::LinkHierarchicalResourceRequest::
                    set_requestor_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "requestor_id",
                &app::dto::hierarchical_resources::LinkHierarchicalResourceRequest::
                    get_requestor_id,
                &app::dto::hierarchical_resources::LinkHierarchicalResourceRequest::
                    set_requestor_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "link_hierarchical_resource_request";
    }
};

template<>
struct serde::Serializable<app::dto::hierarchical_resources::LinkHierarchicalResourceResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "id",
            &app::dto::hierarchical_resources::LinkHierarchicalResourceResponse::get_id,
            &app::dto::hierarchical_resources::LinkHierarchicalResourceResponse::set_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "link_hierarchical_resource_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::hierarchical_resources::tests {
using namespace boost::ut;
suite << app_dto_hierarchical_resources >> app_dto_hierarchical_resources_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<app::dto::hierarchical_resources::
                                                 GenerateHierarchicalResourceRequest>::fields())> >
            0
        );
    };
};
} // namespace app::dto::hierarchical_resources::tests
#endif
