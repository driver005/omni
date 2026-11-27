export module app_payment:poll_handler;

import std;
import core_router;
import app_context;

import :poll_input;
import :context;
import :poll_result;
import :poll_runtime;
import core_behavior;
import interfaces;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_auth;
import app_http;
import app_metrics;
import core_cache;
import core_error;

export namespace app::handlers::poll {

/**
 * Local substitutes for the upstream Cargo features that gate the poll route.
 * Upstream app.rs:2472 registers the scope only under
 * `#[cfg(all(feature = "oltp", feature = "v1"))]`. This port has no Cargo
 * `#[cfg]`, so these constexpr gates are the exact local equivalent. Both
 * features are compiled in here, matching an upstream release build.
 */
inline constexpr bool kOltp = true;
inline constexpr bool kV1 = true;

/**
 * @brief v1 poll route family.
 *
 * Strict upstream-identical port of routes/poll.rs + core/poll.rs with local
 * in-process substitutes for external infrastructure (Redis, processor domain
 * object, ApiEventMetric). Wire shape, auth, errors, key derivation and control
 * flow mirror upstream; see docs/hyperswitch-parity/poll.md for file:line
 * evidence per helper.
 */
class PollHandlersV1
{
public:
    void bind_request_support(PollRequestSupport& support) noexcept
    {
        m_request_support = &support;
    }

    PollRequestSupport* m_request_support{nullptr};

    void bind_policy(PollPolicy& p) noexcept
    {
        m_policy = &p;
    }

    PollPolicy* m_policy{nullptr};

    PollHandlersV1(::app::context::AppContext& context, PollRuntime& runtime) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{context}
    {
    }

    void handle_retrieve_poll_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};

        // 1. request -> Input.
        // routes/poll.rs:37-50 — dynamic auth extractor selection + registration gate.
        const auto auth_error = m_request_support->authorize(
            req,
            "retrieve_poll_status",
            spec_for("retrieve_poll_status")
        );
        if (auth_error) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        // routes/poll.rs:31,34-36 — web::Path<String> -> PollId.
        const auto poll_id = m_input_support.poll_id_from_path(req);
        if (!poll_id.has_value()) {
            const auto& error = poll_id.error();
            error.send(sink);
            return;
        }
        const std::string& request_poll_id = *poll_id;

        // routes/poll.rs:56-57 — auth.platform.get_processor().clone() supplies
        // the processor passed to core::poll::retrieve_poll_status.
        const auto processor_merchant_id =
            m_input_support.resolve_processor_account_id(req, m_policy->last().context);

        // 2. Input -> Result.
        auto result = m_runtime.retrieve_poll_status(m_request_support->context(req));
        result.handle(sink);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // app.rs:2472-2480 — web::scope("/poll") +
        // web::resource("/status/{poll_id}").route(web::get()), gated by
        // all(feature = "oltp", feature = "v1"). No C++-only routes exist.
        if constexpr (kOltp && kV1) {
            Router{router, "poll"}.add_router(
                Router{router, "status"}.add_route(
                    Route{":poll_id"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                interfaces::io::ResponseSink sink{res, std::move(send)};

                                handle_retrieve_poll_status(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch retrieve_poll_status")
                        .description(
                            "Retrieve poll status. Auth: SDK authorization when an "
                            "Authorization header is present, otherwise publishable key "
                            "(connected scope allowed, platform self denied). "
                            "404 PollNotFound when the processor-scoped poll key is absent."
                        )
                )
            );
        }
    }

private:
    ::app::context::AppContext& m_context;
    PollRuntime& m_runtime;
    PollInputSupport m_input_support;
};

/**
 * Facade ("handler of handlers") for the poll domain.
 *
 * Owns the single PollRuntime instance shared by every version partition.
 * all.cppm keeps using this exact class name, ctor and install() signature.
 */
class PollRouteHandlers
{
public:
    explicit PollRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_v1{context, m_runtime},
        m_input_support{context},
        m_policy{context, m_input_support},
        m_request_support{m_policy}
    {
        m_v1.bind_request_support(m_request_support);
        m_v1.bind_policy(m_policy);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v1.install(router);
    }

private:
    PollRuntime m_runtime;
    PollHandlersV1 m_v1;
    PollInputSupport m_input_support;
    PollPolicy m_policy;
    PollRequestSupport m_request_support;
};

} // namespace app::handlers::poll
