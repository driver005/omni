module;
export module app_handlers:metrics;

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
    void install(core::router::RouterContext<>& router)
    {
        using Router = utils::openapi::ApiRouter;
        Router{router, "/metrics"};
    }

    explicit MetricsRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::metrics
