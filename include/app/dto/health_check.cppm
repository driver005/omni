module;
export module app_dto:health_check;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::health_check {

enum class HealthCheckServices
{
    DynamicRoutingService,
};

enum class HealthState
{
    Running,
    Error,
    NotApplicable,
};

class RouterHealthCheckResponse
{
public:
    void set_database(bool value) { database = std::move(value); }
    [[nodiscard]] const bool& get_database() const noexcept { return database; }
    void set_redis(bool value) { redis = std::move(value); }
    [[nodiscard]] const bool& get_redis() const noexcept { return redis; }
    void set_vault(std::optional<bool> value) { vault = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_vault() const noexcept { return vault; }
    void set_analytics(bool value) { analytics = std::move(value); }
    [[nodiscard]] const bool& get_analytics() const noexcept { return analytics; }
    void set_opensearch(bool value) { opensearch = std::move(value); }
    [[nodiscard]] const bool& get_opensearch() const noexcept { return opensearch; }
    void set_outgoing_request(bool value) { outgoing_request = std::move(value); }
    [[nodiscard]] const bool& get_outgoing_request() const noexcept { return outgoing_request; }
    void set_grpc_health_check(std::unordered_map<std::string, bool> value) { grpc_health_check = std::move(value); }
    [[nodiscard]] const std::unordered_map<std::string, bool>& get_grpc_health_check() const noexcept { return grpc_health_check; }
    void set_decision_engine(bool value) { decision_engine = std::move(value); }
    [[nodiscard]] const bool& get_decision_engine() const noexcept { return decision_engine; }
    void set_unified_connector_service(std::optional<bool> value) { unified_connector_service = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_unified_connector_service() const noexcept { return unified_connector_service; }
    bool database{};
    bool redis{};
    std::optional<bool> vault{};
    bool analytics{};
    bool opensearch{};
    bool outgoing_request{};
    std::unordered_map<std::string, bool> grpc_health_check{};
    bool decision_engine{};
    std::optional<bool> unified_connector_service{};
};

class SchedulerHealthCheckResponse
{
public:
    void set_database(bool value) { database = std::move(value); }
    [[nodiscard]] const bool& get_database() const noexcept { return database; }
    void set_redis(bool value) { redis = std::move(value); }
    [[nodiscard]] const bool& get_redis() const noexcept { return redis; }
    void set_outgoing_request(bool value) { outgoing_request = std::move(value); }
    [[nodiscard]] const bool& get_outgoing_request() const noexcept { return outgoing_request; }
    bool database{};
    bool redis{};
    bool outgoing_request{};
};
}

template<>
struct serde::Serializable<app::dto::health_check::RouterHealthCheckResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"database", &app::dto::health_check::RouterHealthCheckResponse::get_database, &app::dto::health_check::RouterHealthCheckResponse::set_database, serde::FieldOptions{}>{},
            serde::FieldDesc<"redis", &app::dto::health_check::RouterHealthCheckResponse::get_redis, &app::dto::health_check::RouterHealthCheckResponse::set_redis, serde::FieldOptions{}>{},
            serde::FieldDesc<"vault", &app::dto::health_check::RouterHealthCheckResponse::get_vault, &app::dto::health_check::RouterHealthCheckResponse::set_vault, serde::FieldOptions{}>{},
            serde::FieldDesc<"analytics", &app::dto::health_check::RouterHealthCheckResponse::get_analytics, &app::dto::health_check::RouterHealthCheckResponse::set_analytics, serde::FieldOptions{}>{},
            serde::FieldDesc<"opensearch", &app::dto::health_check::RouterHealthCheckResponse::get_opensearch, &app::dto::health_check::RouterHealthCheckResponse::set_opensearch, serde::FieldOptions{}>{},
            serde::FieldDesc<"outgoing_request", &app::dto::health_check::RouterHealthCheckResponse::get_outgoing_request, &app::dto::health_check::RouterHealthCheckResponse::set_outgoing_request, serde::FieldOptions{}>{},
            serde::FieldDesc<"grpc_health_check", &app::dto::health_check::RouterHealthCheckResponse::get_grpc_health_check, &app::dto::health_check::RouterHealthCheckResponse::set_grpc_health_check, serde::FieldOptions{}>{},
            serde::FieldDesc<"decision_engine", &app::dto::health_check::RouterHealthCheckResponse::get_decision_engine, &app::dto::health_check::RouterHealthCheckResponse::set_decision_engine, serde::FieldOptions{}>{},
            serde::FieldDesc<"unified_connector_service", &app::dto::health_check::RouterHealthCheckResponse::get_unified_connector_service, &app::dto::health_check::RouterHealthCheckResponse::set_unified_connector_service, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "routerhealthcheckresponse"; }
};

template<>
struct serde::Serializable<app::dto::health_check::SchedulerHealthCheckResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"database", &app::dto::health_check::SchedulerHealthCheckResponse::get_database, &app::dto::health_check::SchedulerHealthCheckResponse::set_database, serde::FieldOptions{}>{},
            serde::FieldDesc<"redis", &app::dto::health_check::SchedulerHealthCheckResponse::get_redis, &app::dto::health_check::SchedulerHealthCheckResponse::set_redis, serde::FieldOptions{}>{},
            serde::FieldDesc<"outgoing_request", &app::dto::health_check::SchedulerHealthCheckResponse::get_outgoing_request, &app::dto::health_check::SchedulerHealthCheckResponse::set_outgoing_request, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "schedulerhealthcheckresponse"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::health_check::tests {
using namespace boost::ut;
suite<"app_dto_health_check"> app_dto_health_check_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::health_check::RouterHealthCheckResponse>::fields())> > 0);
    };
};
}
#endif
