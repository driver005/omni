module;
export module app_dto:superposition_proxy;

import core_serde;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::superposition_proxy {

class AuditLogResponse : public serde::Serializer<AuditLogResponse>
{
public:
    void set_id(std::string value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return id;
    }

    void set_table_name(std::string value)
    {
        table_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_table_name() const noexcept
    {
        return table_name;
    }

    void set_user_name(std::string value)
    {
        user_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_user_name() const noexcept
    {
        return user_name;
    }

    void set_timestamp(std::string value)
    {
        timestamp = std::move(value);
    }

    [[nodiscard]] const std::string& get_timestamp() const noexcept
    {
        return timestamp;
    }

    void set_action(std::string value)
    {
        action = std::move(value);
    }

    [[nodiscard]] const std::string& get_action() const noexcept
    {
        return action;
    }

    void set_original_data(std::optional<serde::Value> value)
    {
        original_data = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_original_data() const noexcept
    {
        return original_data;
    }

    void set_new_data(std::optional<serde::Value> value)
    {
        new_data = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_new_data() const noexcept
    {
        return new_data;
    }

    void set_query(std::string value)
    {
        query = std::move(value);
    }

    [[nodiscard]] const std::string& get_query() const noexcept
    {
        return query;
    }

    std::string id{};
    std::string table_name{};
    std::string user_name{};
    std::string timestamp{};
    std::string action{};
    std::optional<serde::Value> original_data{};
    std::optional<serde::Value> new_data{};
    std::string query{};
};

class ContextResponse : public serde::Serializer<ContextResponse>
{
public:
    void set_id(std::string value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return id;
    }

    void set_value(serde::Value value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_value() const noexcept
    {
        return value;
    }

    void set_override_(serde::Value value)
    {
        override_ = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_override_() const noexcept
    {
        return override_;
    }

    void set_override_id(std::string value)
    {
        override_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_override_id() const noexcept
    {
        return override_id;
    }

    void set_weight(std::string value)
    {
        weight = std::move(value);
    }

    [[nodiscard]] const std::string& get_weight() const noexcept
    {
        return weight;
    }

    void set_description(std::string value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return description;
    }

    void set_change_reason(std::string value)
    {
        change_reason = std::move(value);
    }

    [[nodiscard]] const std::string& get_change_reason() const noexcept
    {
        return change_reason;
    }

    void set_created_at(std::string value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_created_by(std::string value)
    {
        created_by = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_by() const noexcept
    {
        return created_by;
    }

    void set_last_modified_at(std::string value)
    {
        last_modified_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_last_modified_at() const noexcept
    {
        return last_modified_at;
    }

    void set_last_modified_by(std::string value)
    {
        last_modified_by = std::move(value);
    }

    [[nodiscard]] const std::string& get_last_modified_by() const noexcept
    {
        return last_modified_by;
    }

    std::string id{};
    serde::Value value{};
    serde::Value override_{};
    std::string override_id{};
    std::string weight{};
    std::string description{};
    std::string change_reason{};
    std::string created_at{};
    std::string created_by{};
    std::string last_modified_at{};
    std::string last_modified_by{};
};

class DefaultConfigResponse : public serde::Serializer<DefaultConfigResponse>
{
public:
    void set_key(std::string value)
    {
        key = std::move(value);
    }

    [[nodiscard]] const std::string& get_key() const noexcept
    {
        return key;
    }

    void set_value(serde::Value value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_value() const noexcept
    {
        return value;
    }

    void set_schema(serde::Value value)
    {
        schema = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_schema() const noexcept
    {
        return schema;
    }

    void set_description(std::string value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return description;
    }

    void set_change_reason(std::string value)
    {
        change_reason = std::move(value);
    }

    [[nodiscard]] const std::string& get_change_reason() const noexcept
    {
        return change_reason;
    }

    void set_value_validation_function_name(std::optional<std::string> value)
    {
        value_validation_function_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_value_validation_function_name() const noexcept
    {
        return value_validation_function_name;
    }

    void set_value_compute_function_name(std::optional<std::string> value)
    {
        value_compute_function_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_value_compute_function_name() const noexcept
    {
        return value_compute_function_name;
    }

    void set_created_at(std::string value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_created_by(std::string value)
    {
        created_by = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_by() const noexcept
    {
        return created_by;
    }

    void set_last_modified_at(std::string value)
    {
        last_modified_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_last_modified_at() const noexcept
    {
        return last_modified_at;
    }

    void set_last_modified_by(std::string value)
    {
        last_modified_by = std::move(value);
    }

    [[nodiscard]] const std::string& get_last_modified_by() const noexcept
    {
        return last_modified_by;
    }

    std::string key{};
    serde::Value value{};
    serde::Value schema{};
    std::string description{};
    std::string change_reason{};
    std::optional<std::string> value_validation_function_name{};
    std::optional<std::string> value_compute_function_name{};
    std::string created_at{};
    std::string created_by{};
    std::string last_modified_at{};
    std::string last_modified_by{};
};

class DimensionResponse : public serde::Serializer<DimensionResponse>
{
public:
    void set_dimension(std::string value)
    {
        dimension = std::move(value);
    }

    [[nodiscard]] const std::string& get_dimension() const noexcept
    {
        return dimension;
    }

    void set_position(std::int32_t value)
    {
        position = std::move(value);
    }

    [[nodiscard]] const std::int32_t& get_position() const noexcept
    {
        return position;
    }

    void set_schema(serde::Value value)
    {
        schema = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_schema() const noexcept
    {
        return schema;
    }

    void set_value_validation_function_name(std::optional<std::string> value)
    {
        value_validation_function_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_value_validation_function_name() const noexcept
    {
        return value_validation_function_name;
    }

    void set_description(std::string value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return description;
    }

    void set_change_reason(std::string value)
    {
        change_reason = std::move(value);
    }

    [[nodiscard]] const std::string& get_change_reason() const noexcept
    {
        return change_reason;
    }

    void set_last_modified_at(std::string value)
    {
        last_modified_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_last_modified_at() const noexcept
    {
        return last_modified_at;
    }

    void set_last_modified_by(std::string value)
    {
        last_modified_by = std::move(value);
    }

    [[nodiscard]] const std::string& get_last_modified_by() const noexcept
    {
        return last_modified_by;
    }

    void set_created_at(std::string value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_created_by(std::string value)
    {
        created_by = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_by() const noexcept
    {
        return created_by;
    }

    void set_dependency_graph(serde::Value value)
    {
        dependency_graph = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_dependency_graph() const noexcept
    {
        return dependency_graph;
    }

    void set_dimension_type(serde::Value value)
    {
        dimension_type = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_dimension_type() const noexcept
    {
        return dimension_type;
    }

    void set_value_compute_function_name(std::optional<std::string> value)
    {
        value_compute_function_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_value_compute_function_name() const noexcept
    {
        return value_compute_function_name;
    }

    void set_mandatory(bool value)
    {
        mandatory = std::move(value);
    }

    [[nodiscard]] const bool& get_mandatory() const noexcept
    {
        return mandatory;
    }

    std::string dimension{};
    std::int32_t position{};
    serde::Value schema{};
    std::optional<std::string> value_validation_function_name{};
    std::string description{};
    std::string change_reason{};
    std::string last_modified_at{};
    std::string last_modified_by{};
    std::string created_at{};
    std::string created_by{};
    serde::Value dependency_graph{};
    serde::Value dimension_type{};
    std::optional<std::string> value_compute_function_name{};
    bool mandatory{};
};

class PaginatedListResponse : public serde::Serializer<PaginatedListResponse>
{
public:
    void set_total_pages(std::int32_t value)
    {
        total_pages = std::move(value);
    }

    [[nodiscard]] const std::int32_t& get_total_pages() const noexcept
    {
        return total_pages;
    }

    void set_total_items(std::int32_t value)
    {
        total_items = std::move(value);
    }

    [[nodiscard]] const std::int32_t& get_total_items() const noexcept
    {
        return total_items;
    }

    void set_data(std::vector<std::string> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_data() const noexcept
    {
        return data;
    }

    std::int32_t total_pages{};
    std::int32_t total_items{};
    std::vector<std::string> data{};
};

class ResolveExplanationEntry : public serde::Serializer<ResolveExplanationEntry>
{
public:
    void set_context_id(std::string value)
    {
        context_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_context_id() const noexcept
    {
        return context_id;
    }

    void set_condition(serde::Value value)
    {
        condition = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_condition() const noexcept
    {
        return condition;
    }

    void set_override_id(std::string value)
    {
        override_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_override_id() const noexcept
    {
        return override_id;
    }

    void set_value_before(serde::Value value)
    {
        value_before = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_value_before() const noexcept
    {
        return value_before;
    }

    void set_value_after(serde::Value value)
    {
        value_after = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_value_after() const noexcept
    {
        return value_after;
    }

    std::string context_id{};
    serde::Value condition{};
    std::string override_id{};
    serde::Value value_before{};
    serde::Value value_after{};
};

class ResolvedConfigEntry : public serde::Serializer<ResolvedConfigEntry>
{
public:
    void set_description(std::string value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return description;
    }

    void set_schema(serde::Value value)
    {
        schema = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_schema() const noexcept
    {
        return schema;
    }

    void set_value(serde::Value value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const serde::Value& get_value() const noexcept
    {
        return value;
    }

    std::string description{};
    serde::Value schema{};
    serde::Value value{};
};

class ResolveConfigExplanationResponse : public serde::Serializer<ResolveConfigExplanationResponse>
{
public:
    void set_key(std::string value)
    {
        key = std::move(value);
    }

    [[nodiscard]] const std::string& get_key() const noexcept
    {
        return key;
    }

    void set_timeline(std::vector<ResolveExplanationEntry> value)
    {
        timeline = std::move(value);
    }

    [[nodiscard]] const std::vector<ResolveExplanationEntry>& get_timeline() const noexcept
    {
        return timeline;
    }

    std::string key{};
    std::vector<ResolveExplanationEntry> timeline{};
};

class ResolveConfigResponse : public serde::Serializer<ResolveConfigResponse>
{
public:
    void set_value(std::unordered_map<std::string, ResolvedConfigEntry> value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::unordered_map<std::string, ResolvedConfigEntry>&
    get_value() const noexcept
    {
        return value;
    }

    std::unordered_map<std::string, ResolvedConfigEntry> value{};
};
} // namespace app::dto::superposition_proxy

template<>
struct serde::Serializable<app::dto::superposition_proxy::ContextResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::dto::superposition_proxy::ContextResponse::get_id,
                &app::dto::superposition_proxy::ContextResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value",
                &app::dto::superposition_proxy::ContextResponse::get_value,
                &app::dto::superposition_proxy::ContextResponse::set_value,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "override",
                &app::dto::superposition_proxy::ContextResponse::get_override_,
                &app::dto::superposition_proxy::ContextResponse::set_override_,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "override_id",
                &app::dto::superposition_proxy::ContextResponse::get_override_id,
                &app::dto::superposition_proxy::ContextResponse::set_override_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "weight",
                &app::dto::superposition_proxy::ContextResponse::get_weight,
                &app::dto::superposition_proxy::ContextResponse::set_weight,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::dto::superposition_proxy::ContextResponse::get_description,
                &app::dto::superposition_proxy::ContextResponse::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "change_reason",
                &app::dto::superposition_proxy::ContextResponse::get_change_reason,
                &app::dto::superposition_proxy::ContextResponse::set_change_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::dto::superposition_proxy::ContextResponse::get_created_at,
                &app::dto::superposition_proxy::ContextResponse::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_by",
                &app::dto::superposition_proxy::ContextResponse::get_created_by,
                &app::dto::superposition_proxy::ContextResponse::set_created_by,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_modified_at",
                &app::dto::superposition_proxy::ContextResponse::get_last_modified_at,
                &app::dto::superposition_proxy::ContextResponse::set_last_modified_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_modified_by",
                &app::dto::superposition_proxy::ContextResponse::get_last_modified_by,
                &app::dto::superposition_proxy::ContextResponse::set_last_modified_by,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "context_response";
    }
};

template<>
struct serde::Serializable<app::dto::superposition_proxy::DefaultConfigResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "key",
                &app::dto::superposition_proxy::DefaultConfigResponse::get_key,
                &app::dto::superposition_proxy::DefaultConfigResponse::set_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value",
                &app::dto::superposition_proxy::DefaultConfigResponse::get_value,
                &app::dto::superposition_proxy::DefaultConfigResponse::set_value,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "schema",
                &app::dto::superposition_proxy::DefaultConfigResponse::get_schema,
                &app::dto::superposition_proxy::DefaultConfigResponse::set_schema,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::dto::superposition_proxy::DefaultConfigResponse::get_description,
                &app::dto::superposition_proxy::DefaultConfigResponse::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "change_reason",
                &app::dto::superposition_proxy::DefaultConfigResponse::get_change_reason,
                &app::dto::superposition_proxy::DefaultConfigResponse::set_change_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value_validation_function_name",
                &app::dto::superposition_proxy::DefaultConfigResponse::
                    get_value_validation_function_name,
                &app::dto::superposition_proxy::DefaultConfigResponse::
                    set_value_validation_function_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value_compute_function_name",
                &app::dto::superposition_proxy::DefaultConfigResponse::
                    get_value_compute_function_name,
                &app::dto::superposition_proxy::DefaultConfigResponse::
                    set_value_compute_function_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::dto::superposition_proxy::DefaultConfigResponse::get_created_at,
                &app::dto::superposition_proxy::DefaultConfigResponse::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_by",
                &app::dto::superposition_proxy::DefaultConfigResponse::get_created_by,
                &app::dto::superposition_proxy::DefaultConfigResponse::set_created_by,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_modified_at",
                &app::dto::superposition_proxy::DefaultConfigResponse::get_last_modified_at,
                &app::dto::superposition_proxy::DefaultConfigResponse::set_last_modified_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_modified_by",
                &app::dto::superposition_proxy::DefaultConfigResponse::get_last_modified_by,
                &app::dto::superposition_proxy::DefaultConfigResponse::set_last_modified_by,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "default_config_response";
    }
};

template<>
struct serde::Serializable<app::dto::superposition_proxy::DimensionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "dimension",
                &app::dto::superposition_proxy::DimensionResponse::get_dimension,
                &app::dto::superposition_proxy::DimensionResponse::set_dimension,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "position",
                &app::dto::superposition_proxy::DimensionResponse::get_position,
                &app::dto::superposition_proxy::DimensionResponse::set_position,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "schema",
                &app::dto::superposition_proxy::DimensionResponse::get_schema,
                &app::dto::superposition_proxy::DimensionResponse::set_schema,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value_validation_function_name",
                &app::dto::superposition_proxy::DimensionResponse::
                    get_value_validation_function_name,
                &app::dto::superposition_proxy::DimensionResponse::
                    set_value_validation_function_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::dto::superposition_proxy::DimensionResponse::get_description,
                &app::dto::superposition_proxy::DimensionResponse::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "change_reason",
                &app::dto::superposition_proxy::DimensionResponse::get_change_reason,
                &app::dto::superposition_proxy::DimensionResponse::set_change_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_modified_at",
                &app::dto::superposition_proxy::DimensionResponse::get_last_modified_at,
                &app::dto::superposition_proxy::DimensionResponse::set_last_modified_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_modified_by",
                &app::dto::superposition_proxy::DimensionResponse::get_last_modified_by,
                &app::dto::superposition_proxy::DimensionResponse::set_last_modified_by,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::dto::superposition_proxy::DimensionResponse::get_created_at,
                &app::dto::superposition_proxy::DimensionResponse::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_by",
                &app::dto::superposition_proxy::DimensionResponse::get_created_by,
                &app::dto::superposition_proxy::DimensionResponse::set_created_by,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "dependency_graph",
                &app::dto::superposition_proxy::DimensionResponse::get_dependency_graph,
                &app::dto::superposition_proxy::DimensionResponse::set_dependency_graph,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "dimension_type",
                &app::dto::superposition_proxy::DimensionResponse::get_dimension_type,
                &app::dto::superposition_proxy::DimensionResponse::set_dimension_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value_compute_function_name",
                &app::dto::superposition_proxy::DimensionResponse::get_value_compute_function_name,
                &app::dto::superposition_proxy::DimensionResponse::set_value_compute_function_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mandatory",
                &app::dto::superposition_proxy::DimensionResponse::get_mandatory,
                &app::dto::superposition_proxy::DimensionResponse::set_mandatory,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "dimension_response";
    }
};

template<>
struct serde::Serializable<app::dto::superposition_proxy::AuditLogResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::dto::superposition_proxy::AuditLogResponse::get_id,
                &app::dto::superposition_proxy::AuditLogResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "table_name",
                &app::dto::superposition_proxy::AuditLogResponse::get_table_name,
                &app::dto::superposition_proxy::AuditLogResponse::set_table_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "user_name",
                &app::dto::superposition_proxy::AuditLogResponse::get_user_name,
                &app::dto::superposition_proxy::AuditLogResponse::set_user_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "timestamp",
                &app::dto::superposition_proxy::AuditLogResponse::get_timestamp,
                &app::dto::superposition_proxy::AuditLogResponse::set_timestamp,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "action",
                &app::dto::superposition_proxy::AuditLogResponse::get_action,
                &app::dto::superposition_proxy::AuditLogResponse::set_action,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "original_data",
                &app::dto::superposition_proxy::AuditLogResponse::get_original_data,
                &app::dto::superposition_proxy::AuditLogResponse::set_original_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "new_data",
                &app::dto::superposition_proxy::AuditLogResponse::get_new_data,
                &app::dto::superposition_proxy::AuditLogResponse::set_new_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "query",
                &app::dto::superposition_proxy::AuditLogResponse::get_query,
                &app::dto::superposition_proxy::AuditLogResponse::set_query,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "audit_log_response";
    }
};

template<>
struct serde::Serializable<app::dto::superposition_proxy::PaginatedListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "total_pages",
                &app::dto::superposition_proxy::PaginatedListResponse::get_total_pages,
                &app::dto::superposition_proxy::PaginatedListResponse::set_total_pages,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_items",
                &app::dto::superposition_proxy::PaginatedListResponse::get_total_items,
                &app::dto::superposition_proxy::PaginatedListResponse::set_total_items,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::dto::superposition_proxy::PaginatedListResponse::get_data,
                &app::dto::superposition_proxy::PaginatedListResponse::set_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "paginated_list_response";
    }
};

template<>
struct serde::Serializable<app::dto::superposition_proxy::ResolvedConfigEntry>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "description",
                &app::dto::superposition_proxy::ResolvedConfigEntry::get_description,
                &app::dto::superposition_proxy::ResolvedConfigEntry::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "schema",
                &app::dto::superposition_proxy::ResolvedConfigEntry::get_schema,
                &app::dto::superposition_proxy::ResolvedConfigEntry::set_schema,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value",
                &app::dto::superposition_proxy::ResolvedConfigEntry::get_value,
                &app::dto::superposition_proxy::ResolvedConfigEntry::set_value,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "resolved_config_entry";
    }
};

template<>
struct serde::Serializable<app::dto::superposition_proxy::ResolveConfigResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::dto::superposition_proxy::ResolveConfigResponse::get_value,
            &app::dto::superposition_proxy::ResolveConfigResponse::set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "resolve_config_response";
    }
};

template<>
struct serde::Serializable<app::dto::superposition_proxy::ResolveExplanationEntry>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "context_id",
                &app::dto::superposition_proxy::ResolveExplanationEntry::get_context_id,
                &app::dto::superposition_proxy::ResolveExplanationEntry::set_context_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "condition",
                &app::dto::superposition_proxy::ResolveExplanationEntry::get_condition,
                &app::dto::superposition_proxy::ResolveExplanationEntry::set_condition,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "override_id",
                &app::dto::superposition_proxy::ResolveExplanationEntry::get_override_id,
                &app::dto::superposition_proxy::ResolveExplanationEntry::set_override_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value_before",
                &app::dto::superposition_proxy::ResolveExplanationEntry::get_value_before,
                &app::dto::superposition_proxy::ResolveExplanationEntry::set_value_before,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "value_after",
                &app::dto::superposition_proxy::ResolveExplanationEntry::get_value_after,
                &app::dto::superposition_proxy::ResolveExplanationEntry::set_value_after,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "resolve_explanation_entry";
    }
};

template<>
struct serde::Serializable<app::dto::superposition_proxy::ResolveConfigExplanationResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "key",
                &app::dto::superposition_proxy::ResolveConfigExplanationResponse::get_key,
                &app::dto::superposition_proxy::ResolveConfigExplanationResponse::set_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "timeline",
                &app::dto::superposition_proxy::ResolveConfigExplanationResponse::get_timeline,
                &app::dto::superposition_proxy::ResolveConfigExplanationResponse::set_timeline,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "resolve_config_explanation_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::superposition_proxy::tests {
using namespace boost::ut;
suite<"app_dto_superposition_proxy"> app_dto_superposition_proxy_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<decltype(serde::Serializable<
                                       app::dto::superposition_proxy::ContextResponse>::fields())> >
            0
        );
    };
};
} // namespace app::dto::superposition_proxy::tests
#endif
