export module app_handlers:routes;

import std;
import core_router;
import :all;
import app_context;

export namespace app::api {

class AppRoutes
{
public:
    void install(core::router::RouterContext<>& router, AppHandlers& handlers)
    {
        app::context::SerdeSetup{}.install();
        handlers.user().install(router);
        handlers.store().install(router);
        handlers.auth().install(router);
        handlers.routing().install(router);
        handlers.payment().install(router);
        handlers.webhook().install(router);
    }
};

} // namespace app::api
