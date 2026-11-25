export module blog_handlers:model_tier;

import std;
import core_serde;
import core_database;
import :model_member;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

class Tier
{
public:
    Tier() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_name(std::string value)
    {
        m_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    void set_slug(std::string value)
    {
        m_slug = std::move(value);
    }

    [[nodiscard]] const std::string& get_slug() const noexcept
    {
        return m_slug;
    }

    void set_price(double value)
    {
        m_price = std::move(value);
    }

    [[nodiscard]] const double& get_price() const noexcept
    {
        return m_price;
    }

    [[nodiscard]] std::string wire_id() const
    {
        return serde::ghost_id(m_id);
    }

    [[nodiscard]] bool wire_active() const
    {
        return true;
    }

    [[nodiscard]] std::int64_t wire_monthly_price() const
    {
        return static_cast<std::int64_t>(m_price);
    }

    [[nodiscard]] std::int64_t wire_yearly_price() const
    {
        return static_cast<std::int64_t>(m_price * 12);
    }

    [[nodiscard]] std::string wire_type() const
    {
        return m_price > 0 ? "paid" : "free";
    }

    [[nodiscard]] static std::string wire_currency()
    {
        return "usd";
    }

    [[nodiscard]] std::string wire_currency_value() const
    {
        return "usd";
    }

    [[nodiscard]] static std::string wire_visibility()
    {
        return "public";
    }

    [[nodiscard]] std::string wire_visibility_value() const
    {
        return "public";
    }

    [[nodiscard]] serde::RawJson wire_timestamp() const
    {
        return {R"("2024-01-01T00:00:00.000Z")"};
    }

    [[nodiscard]] serde::RawJson wire_null() const
    {
        return {"null"};
    }

    [[nodiscard]] serde::RawJson wire_empty_array() const
    {
        return {"[]"};
    }

    void set_wire_noop(serde::RawJson) {}

    void set_wire_string_noop(std::string) {}

    std::int64_t m_id{};
    std::string m_name{};
    std::string m_slug{};
    double m_price{};
};


} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::Tier>
{
    using M = blog::model::Tier;

    static constexpr auto fields()
    {
        return std::tuple{
            FieldDesc<"id", &M::wire_id, &M::set_id>{},
            FieldDesc<"name", &M::get_name, &M::set_name>{},
            FieldDesc<"slug", &M::get_slug, &M::set_slug>{},
            FieldDesc<"active", &M::wire_active, &M::set_wire_noop>{},
            FieldDesc<"benefits", &M::wire_empty_array, &M::set_wire_noop>{},
            FieldDesc<"currency", &M::wire_currency_value, &M::set_wire_string_noop>{},
            FieldDesc<"description", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"monthly_price", &M::wire_monthly_price, &M::set_wire_noop>{},
            FieldDesc<"yearly_price", &M::wire_yearly_price, &M::set_wire_noop>{},
            FieldDesc<"trial_days", &M::get_id, &M::set_id, FieldOptions{}, zero_normalizer{}>{},
            FieldDesc<"type", &M::wire_type, &M::set_wire_string_noop>{},
            FieldDesc<"visibility", &M::wire_visibility_value, &M::set_wire_string_noop>{},
            FieldDesc<"created_at", &M::wire_timestamp, &M::set_wire_noop>{},
            FieldDesc<"updated_at", &M::wire_timestamp, &M::set_wire_noop>{},
            FieldDesc<"welcome_page_url", &M::wire_null, &M::set_wire_noop>{}
        };
    }

    static constexpr auto db_fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &blog::model::Tier::get_id,
                &blog::model::Tier::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "name",
                &blog::model::Tier::get_name,
                &blog::model::Tier::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "slug",
                &blog::model::Tier::get_slug,
                &blog::model::Tier::set_slug,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "price",
                &blog::model::Tier::get_price,
                &blog::model::Tier::set_price,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "tiers";
    }
};
} // namespace serde


#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;

suite<"blog_model_tier"> blog_model_tier_suite = []
{
    "reflects Tier fields"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Tier>);
        expect(core::db::reflect::field_count_v<Tier> == 4);
        expect(core::db::reflect::EntityMeta<Tier>::table == "tiers");
    };
};

} // namespace blog::model::tests
#endif
