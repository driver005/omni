module;
#include "entity_macros.h"

export module blog_model:offer;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost offers entity — full field set from data/schema/schema.js. */
struct Offer
{
    std::int64_t id{};
    bool active{};
    std::string name{};
    std::string code{};
    std::string product_id{};
    std::string stripe_coupon_id{};
    std::string interval{};
    std::string currency{};
    std::string discount_type{};
    std::int64_t discount_amount{};
    std::string duration{};
    std::int64_t duration_in_months{};
    std::string portal_title{};
    std::string portal_description{};
    std::string redemption_type{};
    std::chrono::system_clock::time_point created_at{};
    std::chrono::system_clock::time_point updated_at{};
};

GHOST_ENTITY(Offer, "offers",
    GHOST_FIELD("id", &Offer::id, core::db::ColumnType::BIGINT, core::db::FieldFlags::PRIMARY_KEY),
    GHOST_FIELD("active", &Offer::active, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("name", &Offer::name, core::db::ColumnType::TEXT),
    GHOST_FIELD("code", &Offer::code, core::db::ColumnType::TEXT),
    GHOST_FIELD("product_id", &Offer::product_id, core::db::ColumnType::TEXT),
    GHOST_FIELD("stripe_coupon_id", &Offer::stripe_coupon_id, core::db::ColumnType::TEXT),
    GHOST_FIELD("interval", &Offer::interval, core::db::ColumnType::TEXT),
    GHOST_FIELD("currency", &Offer::currency, core::db::ColumnType::TEXT),
    GHOST_FIELD("discount_type", &Offer::discount_type, core::db::ColumnType::TEXT),
    GHOST_FIELD("discount_amount", &Offer::discount_amount, core::db::ColumnType::INTEGER),
    GHOST_FIELD("duration", &Offer::duration, core::db::ColumnType::TEXT),
    GHOST_FIELD("duration_in_months", &Offer::duration_in_months, core::db::ColumnType::INTEGER),
    GHOST_FIELD("portal_title", &Offer::portal_title, core::db::ColumnType::TEXT),
    GHOST_FIELD("portal_description", &Offer::portal_description, core::db::ColumnType::TEXT),
    GHOST_FIELD("redemption_type", &Offer::redemption_type, core::db::ColumnType::TEXT),
    GHOST_FIELD("created_at", &Offer::created_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("updated_at", &Offer::updated_at, core::db::ColumnType::TIMESTAMP));

} // namespace blog::model

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
