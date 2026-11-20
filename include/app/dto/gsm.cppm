module;
export module app_dto:gsm;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::gsm {

class ConnectorValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class ErrorCategoryValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class GsmDecisionValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class GsmDeleteRequest
{
public:
    void set_connector(std::string value) { connector = std::move(value); }
    [[nodiscard]] const std::string& get_connector() const noexcept { return connector; }
    void set_flow(std::string value) { flow = std::move(value); }
    [[nodiscard]] const std::string& get_flow() const noexcept { return flow; }
    void set_sub_flow(std::string value) { sub_flow = std::move(value); }
    [[nodiscard]] const std::string& get_sub_flow() const noexcept { return sub_flow; }
    void set_code(std::string value) { code = std::move(value); }
    [[nodiscard]] const std::string& get_code() const noexcept { return code; }
    void set_message(std::string value) { message = std::move(value); }
    [[nodiscard]] const std::string& get_message() const noexcept { return message; }
    std::string connector{};
    std::string flow{};
    std::string sub_flow{};
    std::string code{};
    std::string message{};
};

class GsmDeleteResponse
{
public:
    void set_gsm_rule_delete(bool value) { gsm_rule_delete = std::move(value); }
    [[nodiscard]] const bool& get_gsm_rule_delete() const noexcept { return gsm_rule_delete; }
    void set_connector(std::string value) { connector = std::move(value); }
    [[nodiscard]] const std::string& get_connector() const noexcept { return connector; }
    void set_flow(std::string value) { flow = std::move(value); }
    [[nodiscard]] const std::string& get_flow() const noexcept { return flow; }
    void set_sub_flow(std::string value) { sub_flow = std::move(value); }
    [[nodiscard]] const std::string& get_sub_flow() const noexcept { return sub_flow; }
    void set_code(std::string value) { code = std::move(value); }
    [[nodiscard]] const std::string& get_code() const noexcept { return code; }
    bool gsm_rule_delete{};
    std::string connector{};
    std::string flow{};
    std::string sub_flow{};
    std::string code{};
};

class GsmFeatureDataValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class GsmFeatureValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class StandardisedCodeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class GsmCreateRequest
{
public:
    void set_connector(ConnectorValue value) { connector = std::move(value); }
    [[nodiscard]] const ConnectorValue& get_connector() const noexcept { return connector; }
    void set_flow(std::string value) { flow = std::move(value); }
    [[nodiscard]] const std::string& get_flow() const noexcept { return flow; }
    void set_sub_flow(std::string value) { sub_flow = std::move(value); }
    [[nodiscard]] const std::string& get_sub_flow() const noexcept { return sub_flow; }
    void set_code(std::string value) { code = std::move(value); }
    [[nodiscard]] const std::string& get_code() const noexcept { return code; }
    void set_message(std::string value) { message = std::move(value); }
    [[nodiscard]] const std::string& get_message() const noexcept { return message; }
    void set_status(std::string value) { status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return status; }
    void set_router_error(std::optional<std::string> value) { router_error = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_router_error() const noexcept { return router_error; }
    void set_decision(GsmDecisionValue value) { decision = std::move(value); }
    [[nodiscard]] const GsmDecisionValue& get_decision() const noexcept { return decision; }
    void set_step_up_possible(bool value) { step_up_possible = std::move(value); }
    [[nodiscard]] const bool& get_step_up_possible() const noexcept { return step_up_possible; }
    void set_unified_code(std::optional<std::string> value) { unified_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_unified_code() const noexcept { return unified_code; }
    void set_unified_message(std::optional<std::string> value) { unified_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_unified_message() const noexcept { return unified_message; }
    void set_error_category(std::optional<ErrorCategoryValue> value) { error_category = std::move(value); }
    [[nodiscard]] const std::optional<ErrorCategoryValue>& get_error_category() const noexcept { return error_category; }
    void set_clear_pan_possible(bool value) { clear_pan_possible = std::move(value); }
    [[nodiscard]] const bool& get_clear_pan_possible() const noexcept { return clear_pan_possible; }
    void set_feature(std::optional<GsmFeatureValue> value) { feature = std::move(value); }
    [[nodiscard]] const std::optional<GsmFeatureValue>& get_feature() const noexcept { return feature; }
    void set_feature_data(std::optional<GsmFeatureDataValue> value) { feature_data = std::move(value); }
    [[nodiscard]] const std::optional<GsmFeatureDataValue>& get_feature_data() const noexcept { return feature_data; }
    void set_standardised_code(std::optional<StandardisedCodeValue> value) { standardised_code = std::move(value); }
    [[nodiscard]] const std::optional<StandardisedCodeValue>& get_standardised_code() const noexcept { return standardised_code; }
    void set_description(std::optional<std::string> value) { description = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept { return description; }
    void set_user_guidance_message(std::optional<std::string> value) { user_guidance_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_user_guidance_message() const noexcept { return user_guidance_message; }
    ConnectorValue connector{};
    std::string flow{};
    std::string sub_flow{};
    std::string code{};
    std::string message{};
    std::string status{};
    std::optional<std::string> router_error{};
    GsmDecisionValue decision{};
    bool step_up_possible{};
    std::optional<std::string> unified_code{};
    std::optional<std::string> unified_message{};
    std::optional<ErrorCategoryValue> error_category{};
    bool clear_pan_possible{};
    std::optional<GsmFeatureValue> feature{};
    std::optional<GsmFeatureDataValue> feature_data{};
    std::optional<StandardisedCodeValue> standardised_code{};
    std::optional<std::string> description{};
    std::optional<std::string> user_guidance_message{};
};

class GsmResponse
{
public:
    void set_connector(std::string value) { connector = std::move(value); }
    [[nodiscard]] const std::string& get_connector() const noexcept { return connector; }
    void set_flow(std::string value) { flow = std::move(value); }
    [[nodiscard]] const std::string& get_flow() const noexcept { return flow; }
    void set_sub_flow(std::string value) { sub_flow = std::move(value); }
    [[nodiscard]] const std::string& get_sub_flow() const noexcept { return sub_flow; }
    void set_code(std::string value) { code = std::move(value); }
    [[nodiscard]] const std::string& get_code() const noexcept { return code; }
    void set_message(std::string value) { message = std::move(value); }
    [[nodiscard]] const std::string& get_message() const noexcept { return message; }
    void set_status(std::string value) { status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return status; }
    void set_router_error(std::optional<std::string> value) { router_error = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_router_error() const noexcept { return router_error; }
    void set_decision(GsmDecisionValue value) { decision = std::move(value); }
    [[nodiscard]] const GsmDecisionValue& get_decision() const noexcept { return decision; }
    void set_step_up_possible(bool value) { step_up_possible = std::move(value); }
    [[nodiscard]] const bool& get_step_up_possible() const noexcept { return step_up_possible; }
    void set_unified_code(std::optional<std::string> value) { unified_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_unified_code() const noexcept { return unified_code; }
    void set_unified_message(std::optional<std::string> value) { unified_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_unified_message() const noexcept { return unified_message; }
    void set_error_category(std::optional<ErrorCategoryValue> value) { error_category = std::move(value); }
    [[nodiscard]] const std::optional<ErrorCategoryValue>& get_error_category() const noexcept { return error_category; }
    void set_clear_pan_possible(bool value) { clear_pan_possible = std::move(value); }
    [[nodiscard]] const bool& get_clear_pan_possible() const noexcept { return clear_pan_possible; }
    void set_feature(GsmFeatureValue value) { feature = std::move(value); }
    [[nodiscard]] const GsmFeatureValue& get_feature() const noexcept { return feature; }
    void set_feature_data(std::optional<GsmFeatureDataValue> value) { feature_data = std::move(value); }
    [[nodiscard]] const std::optional<GsmFeatureDataValue>& get_feature_data() const noexcept { return feature_data; }
    void set_standardised_code(std::optional<StandardisedCodeValue> value) { standardised_code = std::move(value); }
    [[nodiscard]] const std::optional<StandardisedCodeValue>& get_standardised_code() const noexcept { return standardised_code; }
    void set_description(std::optional<std::string> value) { description = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept { return description; }
    void set_user_guidance_message(std::optional<std::string> value) { user_guidance_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_user_guidance_message() const noexcept { return user_guidance_message; }
    std::string connector{};
    std::string flow{};
    std::string sub_flow{};
    std::string code{};
    std::string message{};
    std::string status{};
    std::optional<std::string> router_error{};
    GsmDecisionValue decision{};
    bool step_up_possible{};
    std::optional<std::string> unified_code{};
    std::optional<std::string> unified_message{};
    std::optional<ErrorCategoryValue> error_category{};
    bool clear_pan_possible{};
    GsmFeatureValue feature{};
    std::optional<GsmFeatureDataValue> feature_data{};
    std::optional<StandardisedCodeValue> standardised_code{};
    std::optional<std::string> description{};
    std::optional<std::string> user_guidance_message{};
};

class GsmRetrieveRequest
{
public:
    void set_connector(ConnectorValue value) { connector = std::move(value); }
    [[nodiscard]] const ConnectorValue& get_connector() const noexcept { return connector; }
    void set_flow(std::string value) { flow = std::move(value); }
    [[nodiscard]] const std::string& get_flow() const noexcept { return flow; }
    void set_sub_flow(std::string value) { sub_flow = std::move(value); }
    [[nodiscard]] const std::string& get_sub_flow() const noexcept { return sub_flow; }
    void set_code(std::string value) { code = std::move(value); }
    [[nodiscard]] const std::string& get_code() const noexcept { return code; }
    void set_message(std::string value) { message = std::move(value); }
    [[nodiscard]] const std::string& get_message() const noexcept { return message; }
    ConnectorValue connector{};
    std::string flow{};
    std::string sub_flow{};
    std::string code{};
    std::string message{};
};

class GsmUpdateRequest
{
public:
    void set_connector(std::string value) { connector = std::move(value); }
    [[nodiscard]] const std::string& get_connector() const noexcept { return connector; }
    void set_flow(std::string value) { flow = std::move(value); }
    [[nodiscard]] const std::string& get_flow() const noexcept { return flow; }
    void set_sub_flow(std::string value) { sub_flow = std::move(value); }
    [[nodiscard]] const std::string& get_sub_flow() const noexcept { return sub_flow; }
    void set_code(std::string value) { code = std::move(value); }
    [[nodiscard]] const std::string& get_code() const noexcept { return code; }
    void set_message(std::string value) { message = std::move(value); }
    [[nodiscard]] const std::string& get_message() const noexcept { return message; }
    void set_status(std::optional<std::string> value) { status = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_status() const noexcept { return status; }
    void set_router_error(std::optional<std::string> value) { router_error = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_router_error() const noexcept { return router_error; }
    void set_decision(std::optional<GsmDecisionValue> value) { decision = std::move(value); }
    [[nodiscard]] const std::optional<GsmDecisionValue>& get_decision() const noexcept { return decision; }
    void set_step_up_possible(std::optional<bool> value) { step_up_possible = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_step_up_possible() const noexcept { return step_up_possible; }
    void set_unified_code(std::optional<std::string> value) { unified_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_unified_code() const noexcept { return unified_code; }
    void set_unified_message(std::optional<std::string> value) { unified_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_unified_message() const noexcept { return unified_message; }
    void set_error_category(std::optional<ErrorCategoryValue> value) { error_category = std::move(value); }
    [[nodiscard]] const std::optional<ErrorCategoryValue>& get_error_category() const noexcept { return error_category; }
    void set_clear_pan_possible(std::optional<bool> value) { clear_pan_possible = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_clear_pan_possible() const noexcept { return clear_pan_possible; }
    void set_feature(std::optional<GsmFeatureValue> value) { feature = std::move(value); }
    [[nodiscard]] const std::optional<GsmFeatureValue>& get_feature() const noexcept { return feature; }
    void set_feature_data(std::optional<GsmFeatureDataValue> value) { feature_data = std::move(value); }
    [[nodiscard]] const std::optional<GsmFeatureDataValue>& get_feature_data() const noexcept { return feature_data; }
    void set_standardised_code(std::optional<StandardisedCodeValue> value) { standardised_code = std::move(value); }
    [[nodiscard]] const std::optional<StandardisedCodeValue>& get_standardised_code() const noexcept { return standardised_code; }
    void set_description(std::optional<std::string> value) { description = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept { return description; }
    void set_user_guidance_message(std::optional<std::string> value) { user_guidance_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_user_guidance_message() const noexcept { return user_guidance_message; }
    std::string connector{};
    std::string flow{};
    std::string sub_flow{};
    std::string code{};
    std::string message{};
    std::optional<std::string> status{};
    std::optional<std::string> router_error{};
    std::optional<GsmDecisionValue> decision{};
    std::optional<bool> step_up_possible{};
    std::optional<std::string> unified_code{};
    std::optional<std::string> unified_message{};
    std::optional<ErrorCategoryValue> error_category{};
    std::optional<bool> clear_pan_possible{};
    std::optional<GsmFeatureValue> feature{};
    std::optional<GsmFeatureDataValue> feature_data{};
    std::optional<StandardisedCodeValue> standardised_code{};
    std::optional<std::string> description{};
    std::optional<std::string> user_guidance_message{};
};
}

template<>
struct serde::Serializable<app::dto::gsm::ConnectorValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::gsm::ConnectorValue::get_value, &app::dto::gsm::ConnectorValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "connector_value"; }
};

template<>
struct serde::Serializable<app::dto::gsm::GsmDecisionValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::gsm::GsmDecisionValue::get_value, &app::dto::gsm::GsmDecisionValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "gsm_decision_value"; }
};

template<>
struct serde::Serializable<app::dto::gsm::ErrorCategoryValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::gsm::ErrorCategoryValue::get_value, &app::dto::gsm::ErrorCategoryValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "error_category_value"; }
};

template<>
struct serde::Serializable<app::dto::gsm::GsmFeatureValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::gsm::GsmFeatureValue::get_value, &app::dto::gsm::GsmFeatureValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "gsm_feature_value"; }
};

template<>
struct serde::Serializable<app::dto::gsm::GsmFeatureDataValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::gsm::GsmFeatureDataValue::get_value, &app::dto::gsm::GsmFeatureDataValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "gsm_feature_data_value"; }
};

template<>
struct serde::Serializable<app::dto::gsm::StandardisedCodeValue>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &app::dto::gsm::StandardisedCodeValue::get_value, &app::dto::gsm::StandardisedCodeValue::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "standardised_code_value"; }
};

template<>
struct serde::Serializable<app::dto::gsm::GsmCreateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector", &app::dto::gsm::GsmCreateRequest::get_connector, &app::dto::gsm::GsmCreateRequest::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"flow", &app::dto::gsm::GsmCreateRequest::get_flow, &app::dto::gsm::GsmCreateRequest::set_flow, serde::FieldOptions{}>{},
            serde::FieldDesc<"sub_flow", &app::dto::gsm::GsmCreateRequest::get_sub_flow, &app::dto::gsm::GsmCreateRequest::set_sub_flow, serde::FieldOptions{}>{},
            serde::FieldDesc<"code", &app::dto::gsm::GsmCreateRequest::get_code, &app::dto::gsm::GsmCreateRequest::set_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"message", &app::dto::gsm::GsmCreateRequest::get_message, &app::dto::gsm::GsmCreateRequest::set_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::dto::gsm::GsmCreateRequest::get_status, &app::dto::gsm::GsmCreateRequest::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"router_error", &app::dto::gsm::GsmCreateRequest::get_router_error, &app::dto::gsm::GsmCreateRequest::set_router_error, serde::FieldOptions{}>{},
            serde::FieldDesc<"decision", &app::dto::gsm::GsmCreateRequest::get_decision, &app::dto::gsm::GsmCreateRequest::set_decision, serde::FieldOptions{}>{},
            serde::FieldDesc<"step_up_possible", &app::dto::gsm::GsmCreateRequest::get_step_up_possible, &app::dto::gsm::GsmCreateRequest::set_step_up_possible, serde::FieldOptions{}>{},
            serde::FieldDesc<"unified_code", &app::dto::gsm::GsmCreateRequest::get_unified_code, &app::dto::gsm::GsmCreateRequest::set_unified_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"unified_message", &app::dto::gsm::GsmCreateRequest::get_unified_message, &app::dto::gsm::GsmCreateRequest::set_unified_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_category", &app::dto::gsm::GsmCreateRequest::get_error_category, &app::dto::gsm::GsmCreateRequest::set_error_category, serde::FieldOptions{}>{},
            serde::FieldDesc<"clear_pan_possible", &app::dto::gsm::GsmCreateRequest::get_clear_pan_possible, &app::dto::gsm::GsmCreateRequest::set_clear_pan_possible, serde::FieldOptions{}>{},
            serde::FieldDesc<"feature", &app::dto::gsm::GsmCreateRequest::get_feature, &app::dto::gsm::GsmCreateRequest::set_feature, serde::FieldOptions{}>{},
            serde::FieldDesc<"feature_data", &app::dto::gsm::GsmCreateRequest::get_feature_data, &app::dto::gsm::GsmCreateRequest::set_feature_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"standardised_code", &app::dto::gsm::GsmCreateRequest::get_standardised_code, &app::dto::gsm::GsmCreateRequest::set_standardised_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::dto::gsm::GsmCreateRequest::get_description, &app::dto::gsm::GsmCreateRequest::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"user_guidance_message", &app::dto::gsm::GsmCreateRequest::get_user_guidance_message, &app::dto::gsm::GsmCreateRequest::set_user_guidance_message, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "gsm_create_request"; }
};

template<>
struct serde::Serializable<app::dto::gsm::GsmRetrieveRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector", &app::dto::gsm::GsmRetrieveRequest::get_connector, &app::dto::gsm::GsmRetrieveRequest::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"flow", &app::dto::gsm::GsmRetrieveRequest::get_flow, &app::dto::gsm::GsmRetrieveRequest::set_flow, serde::FieldOptions{}>{},
            serde::FieldDesc<"sub_flow", &app::dto::gsm::GsmRetrieveRequest::get_sub_flow, &app::dto::gsm::GsmRetrieveRequest::set_sub_flow, serde::FieldOptions{}>{},
            serde::FieldDesc<"code", &app::dto::gsm::GsmRetrieveRequest::get_code, &app::dto::gsm::GsmRetrieveRequest::set_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"message", &app::dto::gsm::GsmRetrieveRequest::get_message, &app::dto::gsm::GsmRetrieveRequest::set_message, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "gsm_retrieve_request"; }
};

template<>
struct serde::Serializable<app::dto::gsm::GsmUpdateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector", &app::dto::gsm::GsmUpdateRequest::get_connector, &app::dto::gsm::GsmUpdateRequest::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"flow", &app::dto::gsm::GsmUpdateRequest::get_flow, &app::dto::gsm::GsmUpdateRequest::set_flow, serde::FieldOptions{}>{},
            serde::FieldDesc<"sub_flow", &app::dto::gsm::GsmUpdateRequest::get_sub_flow, &app::dto::gsm::GsmUpdateRequest::set_sub_flow, serde::FieldOptions{}>{},
            serde::FieldDesc<"code", &app::dto::gsm::GsmUpdateRequest::get_code, &app::dto::gsm::GsmUpdateRequest::set_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"message", &app::dto::gsm::GsmUpdateRequest::get_message, &app::dto::gsm::GsmUpdateRequest::set_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::dto::gsm::GsmUpdateRequest::get_status, &app::dto::gsm::GsmUpdateRequest::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"router_error", &app::dto::gsm::GsmUpdateRequest::get_router_error, &app::dto::gsm::GsmUpdateRequest::set_router_error, serde::FieldOptions{}>{},
            serde::FieldDesc<"decision", &app::dto::gsm::GsmUpdateRequest::get_decision, &app::dto::gsm::GsmUpdateRequest::set_decision, serde::FieldOptions{}>{},
            serde::FieldDesc<"step_up_possible", &app::dto::gsm::GsmUpdateRequest::get_step_up_possible, &app::dto::gsm::GsmUpdateRequest::set_step_up_possible, serde::FieldOptions{}>{},
            serde::FieldDesc<"unified_code", &app::dto::gsm::GsmUpdateRequest::get_unified_code, &app::dto::gsm::GsmUpdateRequest::set_unified_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"unified_message", &app::dto::gsm::GsmUpdateRequest::get_unified_message, &app::dto::gsm::GsmUpdateRequest::set_unified_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_category", &app::dto::gsm::GsmUpdateRequest::get_error_category, &app::dto::gsm::GsmUpdateRequest::set_error_category, serde::FieldOptions{}>{},
            serde::FieldDesc<"clear_pan_possible", &app::dto::gsm::GsmUpdateRequest::get_clear_pan_possible, &app::dto::gsm::GsmUpdateRequest::set_clear_pan_possible, serde::FieldOptions{}>{},
            serde::FieldDesc<"feature", &app::dto::gsm::GsmUpdateRequest::get_feature, &app::dto::gsm::GsmUpdateRequest::set_feature, serde::FieldOptions{}>{},
            serde::FieldDesc<"feature_data", &app::dto::gsm::GsmUpdateRequest::get_feature_data, &app::dto::gsm::GsmUpdateRequest::set_feature_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"standardised_code", &app::dto::gsm::GsmUpdateRequest::get_standardised_code, &app::dto::gsm::GsmUpdateRequest::set_standardised_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::dto::gsm::GsmUpdateRequest::get_description, &app::dto::gsm::GsmUpdateRequest::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"user_guidance_message", &app::dto::gsm::GsmUpdateRequest::get_user_guidance_message, &app::dto::gsm::GsmUpdateRequest::set_user_guidance_message, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "gsm_update_request"; }
};

template<>
struct serde::Serializable<app::dto::gsm::GsmDeleteRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector", &app::dto::gsm::GsmDeleteRequest::get_connector, &app::dto::gsm::GsmDeleteRequest::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"flow", &app::dto::gsm::GsmDeleteRequest::get_flow, &app::dto::gsm::GsmDeleteRequest::set_flow, serde::FieldOptions{}>{},
            serde::FieldDesc<"sub_flow", &app::dto::gsm::GsmDeleteRequest::get_sub_flow, &app::dto::gsm::GsmDeleteRequest::set_sub_flow, serde::FieldOptions{}>{},
            serde::FieldDesc<"code", &app::dto::gsm::GsmDeleteRequest::get_code, &app::dto::gsm::GsmDeleteRequest::set_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"message", &app::dto::gsm::GsmDeleteRequest::get_message, &app::dto::gsm::GsmDeleteRequest::set_message, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "gsm_delete_request"; }
};

template<>
struct serde::Serializable<app::dto::gsm::GsmDeleteResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"gsm_rule_delete", &app::dto::gsm::GsmDeleteResponse::get_gsm_rule_delete, &app::dto::gsm::GsmDeleteResponse::set_gsm_rule_delete, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &app::dto::gsm::GsmDeleteResponse::get_connector, &app::dto::gsm::GsmDeleteResponse::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"flow", &app::dto::gsm::GsmDeleteResponse::get_flow, &app::dto::gsm::GsmDeleteResponse::set_flow, serde::FieldOptions{}>{},
            serde::FieldDesc<"sub_flow", &app::dto::gsm::GsmDeleteResponse::get_sub_flow, &app::dto::gsm::GsmDeleteResponse::set_sub_flow, serde::FieldOptions{}>{},
            serde::FieldDesc<"code", &app::dto::gsm::GsmDeleteResponse::get_code, &app::dto::gsm::GsmDeleteResponse::set_code, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "gsm_delete_response"; }
};

template<>
struct serde::Serializable<app::dto::gsm::GsmResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector", &app::dto::gsm::GsmResponse::get_connector, &app::dto::gsm::GsmResponse::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"flow", &app::dto::gsm::GsmResponse::get_flow, &app::dto::gsm::GsmResponse::set_flow, serde::FieldOptions{}>{},
            serde::FieldDesc<"sub_flow", &app::dto::gsm::GsmResponse::get_sub_flow, &app::dto::gsm::GsmResponse::set_sub_flow, serde::FieldOptions{}>{},
            serde::FieldDesc<"code", &app::dto::gsm::GsmResponse::get_code, &app::dto::gsm::GsmResponse::set_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"message", &app::dto::gsm::GsmResponse::get_message, &app::dto::gsm::GsmResponse::set_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &app::dto::gsm::GsmResponse::get_status, &app::dto::gsm::GsmResponse::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"router_error", &app::dto::gsm::GsmResponse::get_router_error, &app::dto::gsm::GsmResponse::set_router_error, serde::FieldOptions{}>{},
            serde::FieldDesc<"decision", &app::dto::gsm::GsmResponse::get_decision, &app::dto::gsm::GsmResponse::set_decision, serde::FieldOptions{}>{},
            serde::FieldDesc<"step_up_possible", &app::dto::gsm::GsmResponse::get_step_up_possible, &app::dto::gsm::GsmResponse::set_step_up_possible, serde::FieldOptions{}>{},
            serde::FieldDesc<"unified_code", &app::dto::gsm::GsmResponse::get_unified_code, &app::dto::gsm::GsmResponse::set_unified_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"unified_message", &app::dto::gsm::GsmResponse::get_unified_message, &app::dto::gsm::GsmResponse::set_unified_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_category", &app::dto::gsm::GsmResponse::get_error_category, &app::dto::gsm::GsmResponse::set_error_category, serde::FieldOptions{}>{},
            serde::FieldDesc<"clear_pan_possible", &app::dto::gsm::GsmResponse::get_clear_pan_possible, &app::dto::gsm::GsmResponse::set_clear_pan_possible, serde::FieldOptions{}>{},
            serde::FieldDesc<"feature", &app::dto::gsm::GsmResponse::get_feature, &app::dto::gsm::GsmResponse::set_feature, serde::FieldOptions{}>{},
            serde::FieldDesc<"feature_data", &app::dto::gsm::GsmResponse::get_feature_data, &app::dto::gsm::GsmResponse::set_feature_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"standardised_code", &app::dto::gsm::GsmResponse::get_standardised_code, &app::dto::gsm::GsmResponse::set_standardised_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::dto::gsm::GsmResponse::get_description, &app::dto::gsm::GsmResponse::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"user_guidance_message", &app::dto::gsm::GsmResponse::get_user_guidance_message, &app::dto::gsm::GsmResponse::set_user_guidance_message, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "gsm_response"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::gsm::tests {
using namespace boost::ut;
suite<"app_dto_gsm"> app_dto_gsm_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::gsm::GsmCreateRequest>::fields())> > 0);
    };
};
}
#endif
