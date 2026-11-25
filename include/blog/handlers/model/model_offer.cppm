export module blog_handlers:model_offer;

import std;
import core_serde;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost offers entity — full field set from data/schema/schema.js. */
class Offer
{
public:
    Offer() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_active(bool value)
    {
        m_active = std::move(value);
    }

    [[nodiscard]] const bool& get_active() const noexcept
    {
        return m_active;
    }

    void set_name(std::string value)
    {
        m_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    void set_code(std::string value)
    {
        m_code = std::move(value);
    }

    [[nodiscard]] const std::string& get_code() const noexcept
    {
        return m_code;
    }

    void set_product_id(std::string value)
    {
        m_product_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_product_id() const noexcept
    {
        return m_product_id;
    }

    void set_stripe_coupon_id(std::string value)
    {
        m_stripe_coupon_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_stripe_coupon_id() const noexcept
    {
        return m_stripe_coupon_id;
    }

    void set_interval(std::string value)
    {
        m_interval = std::move(value);
    }

    [[nodiscard]] const std::string& get_interval() const noexcept
    {
        return m_interval;
    }

    void set_currency(std::string value)
    {
        m_currency = std::move(value);
    }

    [[nodiscard]] const std::string& get_currency() const noexcept
    {
        return m_currency;
    }

    void set_discount_type(std::string value)
    {
        m_discount_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_discount_type() const noexcept
    {
        return m_discount_type;
    }

    void set_discount_amount(std::int64_t value)
    {
        m_discount_amount = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_discount_amount() const noexcept
    {
        return m_discount_amount;
    }

    void set_duration(std::string value)
    {
        m_duration = std::move(value);
    }

    [[nodiscard]] const std::string& get_duration() const noexcept
    {
        return m_duration;
    }

    void set_duration_in_months(std::int64_t value)
    {
        m_duration_in_months = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_duration_in_months() const noexcept
    {
        return m_duration_in_months;
    }

    void set_portal_title(std::string value)
    {
        m_portal_title = std::move(value);
    }

    [[nodiscard]] const std::string& get_portal_title() const noexcept
    {
        return m_portal_title;
    }

    void set_portal_description(std::string value)
    {
        m_portal_description = std::move(value);
    }

    [[nodiscard]] const std::string& get_portal_description() const noexcept
    {
        return m_portal_description;
    }

    void set_redemption_type(std::string value)
    {
        m_redemption_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_redemption_type() const noexcept
    {
        return m_redemption_type;
    }

    void set_created_at(std::chrono::system_clock::time_point value)
    {
        m_created_at = std::move(value);
    }

    [[nodiscard]] const std::chrono::system_clock::time_point& get_created_at() const noexcept
    {
        return m_created_at;
    }

    void set_updated_at(std::chrono::system_clock::time_point value)
    {
        m_updated_at = std::move(value);
    }

    [[nodiscard]] const std::chrono::system_clock::time_point& get_updated_at() const noexcept
    {
        return m_updated_at;
    }

    std::int64_t m_id{};
    bool m_active{};
    std::string m_name{};
    std::string m_code{};
    std::string m_product_id{};
    std::string m_stripe_coupon_id{};
    std::string m_interval{};
    std::string m_currency{};
    std::string m_discount_type{};
    std::int64_t m_discount_amount{};
    std::string m_duration{};
    std::int64_t m_duration_in_months{};
    std::string m_portal_title{};
    std::string m_portal_description{};
    std::string m_redemption_type{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
};


} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::Offer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &blog::model::Offer::get_id,
                &blog::model::Offer::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "active",
                &blog::model::Offer::get_active,
                &blog::model::Offer::set_active,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &blog::model::Offer::get_name,
                &blog::model::Offer::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "code",
                &blog::model::Offer::get_code,
                &blog::model::Offer::set_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "product_id",
                &blog::model::Offer::get_product_id,
                &blog::model::Offer::set_product_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "stripe_coupon_id",
                &blog::model::Offer::get_stripe_coupon_id,
                &blog::model::Offer::set_stripe_coupon_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "interval",
                &blog::model::Offer::get_interval,
                &blog::model::Offer::set_interval,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &blog::model::Offer::get_currency,
                &blog::model::Offer::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "discount_type",
                &blog::model::Offer::get_discount_type,
                &blog::model::Offer::set_discount_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "discount_amount",
                &blog::model::Offer::get_discount_amount,
                &blog::model::Offer::set_discount_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "duration",
                &blog::model::Offer::get_duration,
                &blog::model::Offer::set_duration,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "duration_in_months",
                &blog::model::Offer::get_duration_in_months,
                &blog::model::Offer::set_duration_in_months,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "portal_title",
                &blog::model::Offer::get_portal_title,
                &blog::model::Offer::set_portal_title,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "portal_description",
                &blog::model::Offer::get_portal_description,
                &blog::model::Offer::set_portal_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "redemption_type",
                &blog::model::Offer::get_redemption_type,
                &blog::model::Offer::set_redemption_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &blog::model::Offer::get_created_at,
                &blog::model::Offer::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated_at",
                &blog::model::Offer::get_updated_at,
                &blog::model::Offer::set_updated_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "offers";
    }
};
} // namespace serde


#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;
suite<"blog_model_offer"> blog_model_offer_suite = []
{
    "reflects full Ghost offers schema"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Offer>);
        expect(core::db::reflect::field_count_v<Offer> == 17);
        expect(core::db::reflect::EntityMeta<Offer>::table == "offers");
    };
};
} // namespace blog::model::tests
#endif
