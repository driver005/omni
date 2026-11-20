module;
export module app_store:routing;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::routing {

/** @brief Persisted routing configuration mirror of the upstream RoutingConfig api model. */
class RoutingConfigRecord
{
public:
    RoutingConfigRecord() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_config_id(std::string value) { m_config_id = std::move(value); }
    [[nodiscard]] const std::string& get_config_id() const noexcept { return m_config_id; }
    void set_merchant_id(std::string value) { m_merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return m_merchant_id; }
    void set_profile_id(std::string value) { m_profile_id = std::move(value); }
    [[nodiscard]] const std::string& get_profile_id() const noexcept { return m_profile_id; }
    void set_algorithm_id(std::string value) { m_algorithm_id = std::move(value); }
    [[nodiscard]] const std::string& get_algorithm_id() const noexcept { return m_algorithm_id; }
    void set_name(std::string value) { m_name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return m_name; }
    void set_description(std::string value) { m_description = std::move(value); }
    [[nodiscard]] const std::string& get_description() const noexcept { return m_description; }
    void set_algorithm(std::string value) { m_algorithm = std::move(value); }
    [[nodiscard]] const std::string& get_algorithm() const noexcept { return m_algorithm; }
    void set_config_json(std::string value) { m_config_json = std::move(value); }
    [[nodiscard]] const std::string& get_config_json() const noexcept { return m_config_json; }
    void set_created_at(std::string value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::string& get_created_at() const noexcept { return m_created_at; }

    std::int64_t m_id{};
    std::string m_config_id{};
    std::string m_merchant_id{};
    std::string m_profile_id{};
    std::string m_algorithm_id{};
    std::string m_name{};
    std::string m_description{};
    std::string m_algorithm{};
    std::string m_config_json{};
    std::string m_created_at{};
};

} // namespace app::store::routing

template<>
struct serde::Serializable<app::store::routing::RoutingConfigRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &app::store::routing::RoutingConfigRecord::get_id, &app::store::routing::RoutingConfigRecord::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"config_id", &app::store::routing::RoutingConfigRecord::get_config_id, &app::store::routing::RoutingConfigRecord::set_config_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &app::store::routing::RoutingConfigRecord::get_merchant_id, &app::store::routing::RoutingConfigRecord::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &app::store::routing::RoutingConfigRecord::get_profile_id, &app::store::routing::RoutingConfigRecord::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"algorithm_id", &app::store::routing::RoutingConfigRecord::get_algorithm_id, &app::store::routing::RoutingConfigRecord::set_algorithm_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &app::store::routing::RoutingConfigRecord::get_name, &app::store::routing::RoutingConfigRecord::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &app::store::routing::RoutingConfigRecord::get_description, &app::store::routing::RoutingConfigRecord::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"algorithm", &app::store::routing::RoutingConfigRecord::get_algorithm, &app::store::routing::RoutingConfigRecord::set_algorithm, serde::FieldOptions{}>{},
            serde::FieldDesc<"config_json", &app::store::routing::RoutingConfigRecord::get_config_json, &app::store::routing::RoutingConfigRecord::set_config_json, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &app::store::routing::RoutingConfigRecord::get_created_at, &app::store::routing::RoutingConfigRecord::set_created_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "routing_configs"; }
};

#ifdef CONGELADO_TEST
namespace app::store::routing::tests {

using namespace boost::ut;

suite<"app_store_routing_config_record"> routing_config_record_suite = []
{
    "stores routing config fields and reports its table"_test = []
    {
        RoutingConfigRecord record{};
        record.set_config_id("rcfg_test");
        record.set_merchant_id("merchant_test");
        record.set_algorithm("success_rate_based");

        expect(record.get_config_id() == "rcfg_test");
        expect(record.get_merchant_id() == "merchant_test");
        expect(record.get_algorithm() == "success_rate_based");
        expect(serde::Serializable<RoutingConfigRecord>::table_name() == "routing_configs");
    };
};

} // namespace app::store::routing::tests
#endif
