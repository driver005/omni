export module app_handlers_customers:handler;

import std;
import interfaces;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_v1;
import :handlers_v2;

export namespace app::handlers::customers {

/**
 * Facade ("handler of handlers"). The original 813-line handler is split by
 * surface version: v1 `/customers` routes live in `:handlers_v1`, v2
 * `/{prefix}/customers` routes in `:handlers_v2`. Both children share the ONE
 * CustomersRuntime and the ONE CustomersInputSupport owned here, so mutable
 * state is never duplicated across versions.
 */
class CustomersRouteHandlers
{
public:
    explicit CustomersRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{&context},
        m_runtime{context},
        m_input_support{context},
        m_v1{context, m_runtime, m_input_support},
        m_v2{context, m_runtime, m_input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        // Local in-process stores for the fields upstream keeps in dedicated tables.
        if (m_context) {
            m_context->database().register_table("customer_extensions");
            m_context->database().register_table("customer_addresses");
        }
        if constexpr (kV1) {
            m_v1.install(router);
        }
        if constexpr (kV2) {
            std::string base{"/v2/customers"};
            if (m_context) {
                const auto prefix =
                    m_context->config_store().get_value("micro_services", "payment_methods_prefix");
                if (prefix && !prefix->empty()) {
                    base = "/" + *prefix + "/customers";
                }
            }
            if (base != "/customers") {
                m_v2.install(router, base);
            }
        }
    }

private:
    ::app::context::AppContext* m_context{nullptr};
    CustomersRuntime m_runtime;
    CustomersInputSupport m_input_support;
    CustomersHandlersV1 m_v1;
    CustomersHandlersV2 m_v2;
};

} // namespace app::handlers::customers
