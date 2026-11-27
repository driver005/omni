module;
export module app_store:metrics_router;
import core_behavior;

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
    void send_error(interfaces::io::ResponseSink& sink, core::error::AppError error) const
    {
        error.send(sink);
    }

    template<typename T>
    void send_json(interfaces::io::ResponseSink& sink, const T& value, bool ok) const
    {
        sink.set_status(
            ok ? interfaces::io::types::Status::OK : interfaces::io::types::Status::BAD_REQUEST
        );
        sink.set_body(serde::Ser::serialize("application/json", value));
        sink.send();
    }

    ::app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::metrics
