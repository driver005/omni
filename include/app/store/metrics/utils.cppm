module;
export module app_store:metrics_utils;

import std;
import interfaces;
import core_router;
import core_serde;
import core_cache;
import core_error;
import app_context;
import utils_openapi;
import app_model;
import app_dto;

export namespace app::handlers::metrics {

class MetricsRouteHandlers
{
public:
    void install_routes(core::router::RouterContext<>& router)
    {
        using Router = utils::openapi::ApiRouter;
        Router{router, "metrics"};
    }

    explicit MetricsRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{&context}
    {
    }

private:
    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        error.send(res, send);
    }

    template<typename T>
    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        const T& value,
        bool ok
    ) const
    {
        res.set_status(
            ok ? interfaces::io::types::Status::OK : interfaces::io::types::Status::BAD_REQUEST
        );
        res.set_body(serde::Ser::serialize("application/json", value));
        send();
    }

    ::app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::metrics
