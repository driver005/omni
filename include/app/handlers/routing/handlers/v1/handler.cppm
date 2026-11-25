export module app_handlers_routing:handlers_v1;

import std;
import interfaces;
import core_router;
import utils_openapi;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_v1_config_crud;
import :handlers_v1_decision;
import :handlers_v1_surcharge;
import :handlers_v1_migration;

export namespace app::handlers::routing {

/**
 * v1 version facade. Composes family partitions and replays the exact original
 * add_route order (routes 1-29, including the payouts scope). The registry is
 * append-only, so registration order is observable via OpenAPI generation.
 * Runtime and input support are owned once by the root facade; children hold refs.
 */
class RoutingHandlersV1
{
public:
    RoutingHandlersV1(
        app::context::AppContext& context,
        RoutingRuntime& runtime,
        RoutingInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_config_crud{context, runtime, input_support},
        m_decision{context, runtime, input_support},
        m_surcharge{context, runtime, input_support},
        m_migration{context, runtime, input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_config_crud.install_route_1(router);
        m_config_crud.install_route_2(router);
        m_config_crud.install_route_3(router);
        m_config_crud.install_route_4(router);
        m_config_crud.install_route_5(router);
        m_decision.install_route_6(router);
        m_migration.install_route_7(router);
        m_migration.install_route_8(router);
        m_config_crud.install_route_9(router);
        m_decision.install_route_10(router);
        m_decision.install_route_11(router);
        m_decision.install_route_12(router);
        m_surcharge.install_route_13(router);
        m_surcharge.install_route_14(router);
        m_surcharge.install_route_15(router);
        m_decision.install_route_16(router);
        m_decision.install_route_17(router);
        m_config_crud.install_route_18(router);
        m_config_crud.install_route_19(router);
        m_config_crud.install_route_20(router);
        m_config_crud.install_route_21(router);
        m_config_crud.install_route_22(router);
        m_config_crud.install_route_23(router);
        m_decision.install_route_24(router);
        m_decision.install_route_25(router);
        m_config_crud.install_route_26(router);
        m_config_crud.install_route_27(router);
        m_decision.install_route_28(router);
        m_decision.install_route_29(router);
    }

private:
    app::context::AppContext& m_context;
    RoutingRuntime& m_runtime;
    RoutingInputSupport& m_input_support;
    RoutingV1ConfigCrudHandlers m_config_crud;
    RoutingV1DecisionHandlers m_decision;
    RoutingV1SurchargeHandlers m_surcharge;
    RoutingV1MigrationHandlers m_migration;
};

} // namespace app::handlers::routing
