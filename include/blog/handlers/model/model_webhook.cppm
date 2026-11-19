export module blog_handlers:model_webhook;

import std;
import core_serde;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost webhooks entity — full field set from data/schema/schema.js. */
class Webhook
{
public:
    Webhook() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_event(std::string value) { m_event = std::move(value); }
    [[nodiscard]] const std::string& get_event() const noexcept { return m_event; }
    void set_target_url(std::string value) { m_target_url = std::move(value); }
    [[nodiscard]] const std::string& get_target_url() const noexcept { return m_target_url; }
    void set_name(std::string value) { m_name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return m_name; }
    void set_secret(std::string value) { m_secret = std::move(value); }
    [[nodiscard]] const std::string& get_secret() const noexcept { return m_secret; }
    void set_api_version(std::string value) { m_api_version = std::move(value); }
    [[nodiscard]] const std::string& get_api_version() const noexcept { return m_api_version; }
    void set_integration_id(std::string value) { m_integration_id = std::move(value); }
    [[nodiscard]] const std::string& get_integration_id() const noexcept { return m_integration_id; }
    void set_last_triggered_at(std::chrono::system_clock::time_point value) { m_last_triggered_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_last_triggered_at() const noexcept { return m_last_triggered_at; }
    void set_last_triggered_status(std::string value) { m_last_triggered_status = std::move(value); }
    [[nodiscard]] const std::string& get_last_triggered_status() const noexcept { return m_last_triggered_status; }
    void set_last_triggered_error(std::string value) { m_last_triggered_error = std::move(value); }
    [[nodiscard]] const std::string& get_last_triggered_error() const noexcept { return m_last_triggered_error; }
    void set_created_at(std::chrono::system_clock::time_point value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    void set_updated_at(std::chrono::system_clock::time_point value) { m_updated_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }

    std::int64_t m_id{};
    std::string m_event{};
    std::string m_target_url{};
    std::string m_name{};
    std::string m_secret{};
    std::string m_api_version{};
    std::string m_integration_id{};
    std::chrono::system_clock::time_point m_last_triggered_at{};
    std::string m_last_triggered_status{};
    std::string m_last_triggered_error{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
};




} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::Webhook>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::model::Webhook::get_id, &blog::model::Webhook::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"event", &blog::model::Webhook::get_event, &blog::model::Webhook::set_event, serde::FieldOptions{}>{},
            serde::FieldDesc<"target_url", &blog::model::Webhook::get_target_url, &blog::model::Webhook::set_target_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &blog::model::Webhook::get_name, &blog::model::Webhook::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"secret", &blog::model::Webhook::get_secret, &blog::model::Webhook::set_secret, serde::FieldOptions{}>{},
            serde::FieldDesc<"api_version", &blog::model::Webhook::get_api_version, &blog::model::Webhook::set_api_version, serde::FieldOptions{}>{},
            serde::FieldDesc<"integration_id", &blog::model::Webhook::get_integration_id, &blog::model::Webhook::set_integration_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"last_triggered_at", &blog::model::Webhook::get_last_triggered_at, &blog::model::Webhook::set_last_triggered_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"last_triggered_status", &blog::model::Webhook::get_last_triggered_status, &blog::model::Webhook::set_last_triggered_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"last_triggered_error", &blog::model::Webhook::get_last_triggered_error, &blog::model::Webhook::set_last_triggered_error, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &blog::model::Webhook::get_created_at, &blog::model::Webhook::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"updated_at", &blog::model::Webhook::get_updated_at, &blog::model::Webhook::set_updated_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "webhooks"; }
};
} // namespace serde


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
