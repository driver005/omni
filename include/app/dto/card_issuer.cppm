module;
export module app_dto:card_issuer;

import core_serde;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::card_issuer {

class CardIssuerIdValue : public serde::Serializer<CardIssuerIdValue>
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

class CardIssuerNameValue : public serde::Serializer<CardIssuerNameValue>
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

class CardIssuerDeleteRequest : public serde::Serializer<CardIssuerDeleteRequest>
{
public:
    void set_id(CardIssuerIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const CardIssuerIdValue& get_id() const noexcept
    {
        return id;
    }

    CardIssuerIdValue id{};
};

class CardIssuerDeleteResponse : public serde::Serializer<CardIssuerDeleteResponse>
{
public:
    void set_id(CardIssuerIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const CardIssuerIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_deleted(bool value)
    {
        deleted = std::move(value);
    }

    [[nodiscard]] const bool& get_deleted() const noexcept
    {
        return deleted;
    }

    CardIssuerIdValue id{};
    bool deleted{};
};

class CardIssuerRequest : public serde::Serializer<CardIssuerRequest>
{
public:
    void set_issuer_name(CardIssuerNameValue value)
    {
        issuer_name = std::move(value);
    }

    [[nodiscard]] const CardIssuerNameValue& get_issuer_name() const noexcept
    {
        return issuer_name;
    }

    CardIssuerNameValue issuer_name{};
};

class CardIssuerResponse : public serde::Serializer<CardIssuerResponse>
{
public:
    void set_id(CardIssuerIdValue value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const CardIssuerIdValue& get_id() const noexcept
    {
        return id;
    }

    void set_issuer_name(CardIssuerNameValue value)
    {
        issuer_name = std::move(value);
    }

    [[nodiscard]] const CardIssuerNameValue& get_issuer_name() const noexcept
    {
        return issuer_name;
    }

    CardIssuerIdValue id{};
    CardIssuerNameValue issuer_name{};
};

class CardIssuerUpdateRequest : public serde::Serializer<CardIssuerUpdateRequest>
{
public:
    void set_issuer_name(CardIssuerNameValue value)
    {
        issuer_name = std::move(value);
    }

    [[nodiscard]] const CardIssuerNameValue& get_issuer_name() const noexcept
    {
        return issuer_name;
    }

    CardIssuerNameValue issuer_name{};
};

class CardIssuerListResponse : public serde::Serializer<CardIssuerListResponse>
{
public:
    void set_issuers(std::vector<CardIssuerResponse> value)
    {
        issuers = std::move(value);
    }

    [[nodiscard]] const std::vector<CardIssuerResponse>& get_issuers() const noexcept
    {
        return issuers;
    }

    std::vector<CardIssuerResponse> issuers{};
};
} // namespace app::dto::card_issuer

template<>
struct serde::Serializable<app::dto::card_issuer::CardIssuerRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "issuer_name",
            &app::dto::card_issuer::CardIssuerRequest::get_issuer_name,
            &app::dto::card_issuer::CardIssuerRequest::set_issuer_name,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "card_issuer_request";
    }
};

template<>
struct serde::Serializable<app::dto::card_issuer::CardIssuerResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::dto::card_issuer::CardIssuerResponse::get_id,
                &app::dto::card_issuer::CardIssuerResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_name",
                &app::dto::card_issuer::CardIssuerResponse::get_issuer_name,
                &app::dto::card_issuer::CardIssuerResponse::set_issuer_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_issuer_response";
    }
};

template<>
struct serde::Serializable<app::dto::card_issuer::CardIssuerListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "issuers",
            &app::dto::card_issuer::CardIssuerListResponse::get_issuers,
            &app::dto::card_issuer::CardIssuerListResponse::set_issuers,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "card_issuer_list_response";
    }
};

template<>
struct serde::Serializable<app::dto::card_issuer::CardIssuerUpdateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "issuer_name",
            &app::dto::card_issuer::CardIssuerUpdateRequest::get_issuer_name,
            &app::dto::card_issuer::CardIssuerUpdateRequest::set_issuer_name,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "card_issuer_update_request";
    }
};

template<>
struct serde::Serializable<app::dto::card_issuer::CardIssuerDeleteRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "id",
            &app::dto::card_issuer::CardIssuerDeleteRequest::get_id,
            &app::dto::card_issuer::CardIssuerDeleteRequest::set_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "card_issuer_delete_request";
    }
};

template<>
struct serde::Serializable<app::dto::card_issuer::CardIssuerDeleteResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::dto::card_issuer::CardIssuerDeleteResponse::get_id,
                &app::dto::card_issuer::CardIssuerDeleteResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "deleted",
                &app::dto::card_issuer::CardIssuerDeleteResponse::get_deleted,
                &app::dto::card_issuer::CardIssuerDeleteResponse::set_deleted,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_issuer_delete_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::card_issuer::tests {
using namespace boost::ut;
suite<"app_dto_card_issuer"> app_dto_card_issuer_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<app::dto::card_issuer::CardIssuerRequest>::fields())> >
            0
        );
    };
};
} // namespace app::dto::card_issuer::tests
#endif
