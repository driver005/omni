module;
export module app_store:lock_utils_utils;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;

export namespace app::handlers::lock_utils {

class LockUtilsRouteHandlers
{
public:
    explicit LockUtilsRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{&context}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        using Router = utils::openapi::ApiRouter;
        Router{router, "lock_utils"};
    }

private:
    void send_error(
        interfaces::io::ResponseSink& sink,
        core::error::AppError error
    ) const
    {
        error.send(sink);
    }

    void send_json(
        interfaces::io::ResponseSink& sink,
        const serde::Value& value,
        bool ok
    ) const
    {
        sink.set_status(
            ok ? interfaces::io::types::Status::OK : interfaces::io::types::Status::BAD_REQUEST
        );
        sink.set_body(serde::Ser::serialize("application/json", value));
        sink.send();
    }

    ::app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::lock_utils
