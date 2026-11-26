export module app_payment:cards_info_result;

import std;
import core_serde;
import app_model;

export namespace app::handlers::cards_info {

// The card-info responses reuse the exact DTO fields the old core encoded, so
// field names, order and wrapper types stay byte-identical. The Serializable
// specializations below reuse the DTO's FieldDesc tuple; those descriptors
// apply to the derived result object just as they do to the base object.
class CardIinInfoResult : public ::app::model::cards_info::CardInfoResponse
{
public:
    CardIinInfoResult() = default;

    explicit CardIinInfoResult(::app::model::cards_info::CardInfoResponse value) noexcept :
        CardInfoResponse{std::move(value)}
    {
    }
};

class CreateCardsInfoResult : public ::app::model::cards_info::CardInfoResponse
{
public:
    CreateCardsInfoResult() = default;

    explicit CreateCardsInfoResult(::app::model::cards_info::CardInfoResponse value) noexcept :
        CardInfoResponse{std::move(value)}
    {
    }
};

class UpdateCardsInfoResult : public ::app::model::cards_info::CardInfoResponse
{
public:
    UpdateCardsInfoResult() = default;

    explicit UpdateCardsInfoResult(::app::model::cards_info::CardInfoResponse value) noexcept :
        CardInfoResponse{std::move(value)}
    {
    }
};

// The migrate endpoint returns a top-level JSON array of migration records.
// Carry the typed vector; the handler serializes get_items() so the wire stays
// a bare array without any hand-built JSON text.
class MigrateCardsInfoResult
{
public:
    MigrateCardsInfoResult() = default;

    explicit MigrateCardsInfoResult(
        std::vector<::app::model::cards_info::CardInfoMigrationResponse> items
    ) noexcept :
        m_items{std::move(items)}
    {
    }

    [[nodiscard]] const std::vector<::app::model::cards_info::CardInfoMigrationResponse>&
    get_items() const noexcept
    {
        return m_items;
    }

    void set_items(std::vector<::app::model::cards_info::CardInfoMigrationResponse> items)
    {
        m_items = std::move(items);
    }

private:
    std::vector<::app::model::cards_info::CardInfoMigrationResponse> m_items{};
};

} // namespace app::handlers::cards_info

template<>
struct serde::Serializable<::app::handlers::cards_info::CardIinInfoResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::cards_info::CardInfoResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "card_iin_info_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::cards_info::CreateCardsInfoResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::cards_info::CardInfoResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "create_cards_info_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::cards_info::UpdateCardsInfoResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::cards_info::CardInfoResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "update_cards_info_result";
    }
};
