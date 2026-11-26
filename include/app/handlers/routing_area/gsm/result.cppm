export module app_routing:gsm_result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::gsm {

// The four wire responses are plain DTOs built by the old core and encoded with
// Ser::encode_json. Deriving from the exact DTO (and reusing its FieldDesc tuple
// in the Serializable specialization) keeps field names, order and optional
// handling byte-identical when the handler serializes the result.

class CreateGsmRuleResult : public ::app::dto::gsm::GsmResponse
{
public:
    CreateGsmRuleResult() = default;

    explicit CreateGsmRuleResult(::app::dto::gsm::GsmResponse value) noexcept :
        GsmResponse{std::move(value)}
    {
    }
};

class GetGsmRuleResult : public ::app::dto::gsm::GsmResponse
{
public:
    GetGsmRuleResult() = default;

    explicit GetGsmRuleResult(::app::dto::gsm::GsmResponse value) noexcept :
        GsmResponse{std::move(value)}
    {
    }
};

class UpdateGsmRuleResult : public ::app::dto::gsm::GsmResponse
{
public:
    UpdateGsmRuleResult() = default;

    explicit UpdateGsmRuleResult(::app::dto::gsm::GsmResponse value) noexcept :
        GsmResponse{std::move(value)}
    {
    }
};

class DeleteGsmRuleResult : public ::app::dto::gsm::GsmDeleteResponse
{
public:
    DeleteGsmRuleResult() = default;

    explicit DeleteGsmRuleResult(::app::dto::gsm::GsmDeleteResponse value) noexcept :
        GsmDeleteResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::gsm

template<>
struct serde::Serializable<::app::handlers::gsm::CreateGsmRuleResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::dto::gsm::GsmResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "create_gsm_rule_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::gsm::GetGsmRuleResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::dto::gsm::GsmResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "get_gsm_rule_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::gsm::UpdateGsmRuleResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::dto::gsm::GsmResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "update_gsm_rule_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::gsm::DeleteGsmRuleResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::dto::gsm::GsmDeleteResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "delete_gsm_rule_result";
    }
};
