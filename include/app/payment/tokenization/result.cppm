export module app_payment:tokenization_result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::tokenization {

// The two responses inherit their wire fields from the exact DTO the old core built
// and encoded, so field names, order and wrapper types stay byte-identical. The
// Serializable specialization reuses the DTO's FieldDesc tuple; those descriptors
// apply to the derived object just as they do to the base object.

class CreateTokenVaultResult : public ::app::dto::tokenization::GenericTokenizationResponse
{
public:
    CreateTokenVaultResult() = default;

    explicit CreateTokenVaultResult(
        ::app::dto::tokenization::GenericTokenizationResponse value
    ) noexcept :
        GenericTokenizationResponse{std::move(value)}
    {
    }
};

class DeleteTokenizedDataResult : public ::app::dto::tokenization::DeleteTokenDataResponse
{
public:
    DeleteTokenizedDataResult() = default;

    explicit DeleteTokenizedDataResult(
        ::app::dto::tokenization::DeleteTokenDataResponse value
    ) noexcept :
        DeleteTokenDataResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::tokenization

template<>
struct serde::Serializable<::app::handlers::tokenization::CreateTokenVaultResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::dto::tokenization::GenericTokenizationResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "create_token_vault_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::tokenization::DeleteTokenizedDataResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::dto::tokenization::DeleteTokenDataResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "delete_tokenized_data_result";
    }
};
