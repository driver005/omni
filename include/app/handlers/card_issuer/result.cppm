export module app_handlers_card_issuer:result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::card_issuer {

// Each result derives from the exact DTO the old core built and encoded, so
// field names, order and wrapper types stay byte-identical. The Serializable
// specialization reuses the DTO's FieldDesc tuple; those descriptors apply to
// the derived object just as they do to the base object.

class CardIssuerAddResult : public app::dto::card_issuer::CardIssuerResponse
{
public:
    CardIssuerAddResult() = default;

    explicit CardIssuerAddResult(app::dto::card_issuer::CardIssuerResponse value) noexcept :
        CardIssuerResponse{std::move(value)}
    {
    }
};

class CardIssuerUpdateResult : public app::dto::card_issuer::CardIssuerResponse
{
public:
    CardIssuerUpdateResult() = default;

    explicit CardIssuerUpdateResult(app::dto::card_issuer::CardIssuerResponse value) noexcept :
        CardIssuerResponse{std::move(value)}
    {
    }
};

class CardIssuerDeleteResult : public app::dto::card_issuer::CardIssuerDeleteResponse
{
public:
    CardIssuerDeleteResult() = default;

    explicit CardIssuerDeleteResult(app::dto::card_issuer::CardIssuerDeleteResponse value) noexcept
        :
        CardIssuerDeleteResponse{std::move(value)}
    {
    }
};

class CardIssuerListResult : public app::dto::card_issuer::CardIssuerListResponse
{
public:
    CardIssuerListResult() = default;

    explicit CardIssuerListResult(app::dto::card_issuer::CardIssuerListResponse value) noexcept :
        CardIssuerListResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::card_issuer

template<>
struct serde::Serializable<app::handlers::card_issuer::CardIssuerAddResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::card_issuer::CardIssuerResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "card_issuer_add_result";
    }
};

template<>
struct serde::Serializable<app::handlers::card_issuer::CardIssuerUpdateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::card_issuer::CardIssuerResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "card_issuer_update_result";
    }
};

template<>
struct serde::Serializable<app::handlers::card_issuer::CardIssuerDeleteResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::card_issuer::CardIssuerDeleteResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "card_issuer_delete_result";
    }
};

template<>
struct serde::Serializable<app::handlers::card_issuer::CardIssuerListResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::card_issuer::CardIssuerListResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "card_issuer_list_result";
    }
};
