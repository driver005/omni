module;
export module app_dto:offer_engine;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::offer_engine {

/**
 * @brief Scalar wrappers kept for source compatibility with older callers.
 *
 * They are NOT used by the wire DTOs below: upstream serializes `Currency` and
 * `PrimitiveDateTime` as plain JSON strings (types.rs:373-392,
 * api_models/offer_engine.rs:35-55), so `BrowseOffer::currency` and
 * `BrowseOffer::valid_till` now hold `std::optional<std::string>` directly.
 */
class CurrencyValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

enum class OfferEngineCredentialSource
{
    None,
    Application,
    Merchant,
};

class PrimitiveDateTimeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

/** @brief Upstream `api_models::offer_engine::BrowseOffer` (offer_engine.rs:33-55). */
class BrowseOffer
{
public:
    void set_code(std::string value) { code = std::move(value); }
    [[nodiscard]] const std::string& get_code() const noexcept { return code; }
    void set_title(std::optional<std::string> value) { title = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_title() const noexcept { return title; }
    void set_display_title(std::optional<std::string> value) { display_title = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_display_title() const noexcept { return display_title; }
    void set_description(std::optional<std::string> value) { description = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept { return description; }
    // Plain strings on the wire: upstream `Option<Currency>` / `Option<PrimitiveDateTime>`
    // serialize via serde as a JSON string or null, never as an object `{"value":...}`.
    void set_currency(std::optional<std::string> value) { currency = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_currency() const noexcept { return currency; }
    void set_valid_till(std::optional<std::string> value) { valid_till = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_valid_till() const noexcept { return valid_till; }
    std::string code{};
    std::optional<std::string> title{};
    std::optional<std::string> display_title{};
    std::optional<std::string> description{};
    std::optional<std::string> currency{};
    std::optional<std::string> valid_till{};
};

class OfferPaymentInfo
{
public:
    void set_currency(std::string value) { currency = std::move(value); }
    [[nodiscard]] const std::string& get_currency() const noexcept { return currency; }
    std::string currency{};
};

class BrowseOffersRequest
{
public:
    void set_offer_payment_info(std::optional<OfferPaymentInfo> value) { offer_payment_info = std::move(value); }
    [[nodiscard]] const std::optional<OfferPaymentInfo>& get_offer_payment_info() const noexcept { return offer_payment_info; }
    std::optional<OfferPaymentInfo> offer_payment_info{};
};

class BrowseOffersResponse
{
public:
    void set_offers(std::vector<BrowseOffer> value) { offers = std::move(value); }
    [[nodiscard]] const std::vector<BrowseOffer>& get_offers() const noexcept { return offers; }
    std::vector<BrowseOffer> offers{};
};
} // namespace app::dto::offer_engine

template<>
struct serde::Serializable<app::dto::offer_engine::OfferPaymentInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"currency", &app::dto::offer_engine::OfferPaymentInfo::get_currency, &app::dto::offer_engine::OfferPaymentInfo::set_currency, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "offer_payment_info"; }
};

template<>
struct serde::Serializable<app::dto::offer_engine::BrowseOffer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"code", &app::dto::offer_engine::BrowseOffer::get_code, &app::dto::offer_engine::BrowseOffer::set_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"title", &app::dto::offer_engine::BrowseOffer::get_title, &app::dto::offer_engine::BrowseOffer::set_title, serde::FieldOptions{}>{},
            serde::FieldDesc<"display_title", &app::dto::offer_engine::BrowseOffer::get_display_title, &app::dto::offer_engine::BrowseOffer::set_display_title, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::dto::offer_engine::BrowseOffer::get_description, &app::dto::offer_engine::BrowseOffer::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::dto::offer_engine::BrowseOffer::get_currency, &app::dto::offer_engine::BrowseOffer::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"valid_till", &app::dto::offer_engine::BrowseOffer::get_valid_till, &app::dto::offer_engine::BrowseOffer::set_valid_till, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "browse_offer"; }
};

template<>
struct serde::Serializable<app::dto::offer_engine::BrowseOffersRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"offer_payment_info", &app::dto::offer_engine::BrowseOffersRequest::get_offer_payment_info, &app::dto::offer_engine::BrowseOffersRequest::set_offer_payment_info, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "browse_offers_request"; }
};

template<>
struct serde::Serializable<app::dto::offer_engine::BrowseOffersResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"offers", &app::dto::offer_engine::BrowseOffersResponse::get_offers, &app::dto::offer_engine::BrowseOffersResponse::set_offers, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "browse_offers_response"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::offer_engine::tests {
using namespace boost::ut;
suite<"app_dto_offer_engine"> app_dto_offer_engine_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::offer_engine::BrowseOffersRequest>::fields())> > 0);
    };
};
}
#endif
