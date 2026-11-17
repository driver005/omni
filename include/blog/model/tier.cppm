module;
#include "entity_macros.h"

export module blog_model:tier;

import std;
import core_serde;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

class Tier
{
public:
    Tier() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_name(std::string value) { m_name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return m_name; }
    void set_slug(std::string value) { m_slug = std::move(value); }
    [[nodiscard]] const std::string& get_slug() const noexcept { return m_slug; }
    void set_price(double value) { m_price = std::move(value); }
    [[nodiscard]] const double& get_price() const noexcept { return m_price; }

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
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::model::Tier::get_id, &blog::model::Tier::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"name", &blog::model::Tier::get_name, &blog::model::Tier::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"slug", &blog::model::Tier::get_slug, &blog::model::Tier::set_slug, serde::FieldOptions{}>{},
            serde::FieldDesc<"price", &blog::model::Tier::get_price, &blog::model::Tier::set_price, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "tiers"; }
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
