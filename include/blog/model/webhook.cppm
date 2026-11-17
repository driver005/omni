module;
#include "entity_macros.h"

export module blog_model:webhook;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost webhooks entity — full field set from data/schema/schema.js. */
struct Webhook
{
    std::int64_t id{};
    std::string event{};
    std::string target_url{};
    std::string name{};
    std::string secret{};
    std::string api_version{};
    std::string integration_id{};
    std::chrono::system_clock::time_point last_triggered_at{};
    std::string last_triggered_status{};
    std::string last_triggered_error{};
    std::chrono::system_clock::time_point created_at{};
    std::chrono::system_clock::time_point updated_at{};
};

GHOST_ENTITY(Webhook, "webhooks",
    GHOST_FIELD("id", &Webhook::id, core::db::ColumnType::BIGINT, core::db::FieldFlags::PRIMARY_KEY),
    GHOST_FIELD("event", &Webhook::event, core::db::ColumnType::TEXT),
    GHOST_FIELD("target_url", &Webhook::target_url, core::db::ColumnType::TEXT),
    GHOST_FIELD("name", &Webhook::name, core::db::ColumnType::TEXT),
    GHOST_FIELD("secret", &Webhook::secret, core::db::ColumnType::TEXT),
    GHOST_FIELD("api_version", &Webhook::api_version, core::db::ColumnType::TEXT),
    GHOST_FIELD("integration_id", &Webhook::integration_id, core::db::ColumnType::TEXT),
    GHOST_FIELD("last_triggered_at", &Webhook::last_triggered_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("last_triggered_status", &Webhook::last_triggered_status, core::db::ColumnType::TEXT),
    GHOST_FIELD("last_triggered_error", &Webhook::last_triggered_error, core::db::ColumnType::TEXT),
    GHOST_FIELD("created_at", &Webhook::created_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("updated_at", &Webhook::updated_at, core::db::ColumnType::TIMESTAMP));

} // namespace blog::model

#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;
suite<"blog_model_webhook"> blog_model_webhook_suite = []
{
    "reflects full Ghost webhooks schema"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Webhook>);
        expect(core::db::reflect::field_count_v<Webhook> == 12);
        expect(core::db::reflect::EntityMeta<Webhook>::table == "webhooks");
    };
};
} // namespace blog::model::tests
#endif
