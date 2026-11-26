export module app_routing:feature_matrix_handler;

import std;
import interfaces;
import core_router;
import app_context;

import :feature_matrix_input;
import :feature_matrix_result;
import :feature_matrix_runtime;
import :feature_matrix_handlers_v1;

export namespace app::handlers::feature_matrix {

/**
 * @brief Facade ("handler of handlers").
 *
 * Owns the ONE shared `FeatureMatrixRuntime`; the v1 partition holds a reference
 * to it, never its own instance. All upstream registrations for this domain are
 * v1 (`all(feature = "olap", feature = "v1")`, app.rs:3430), so there is no v2
 * scope and no v2 partition. The public class name, ctor `(AppContext&)` and
 * `install(router)` are unchanged; `all.cppm`/`routes.cppm` need no edits.
 */
class FeatureMatrixRouteHandlers
{
public:
    explicit FeatureMatrixRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_v1{context, m_runtime}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v1.install_v1(router);
    }

private:
    ::app::context::AppContext& m_context;
    FeatureMatrixRuntime m_runtime; // SINGLE runtime — no duplicated mutable state.
    FeatureMatrixHandlersV1 m_v1;   // Holds AppContext& + FeatureMatrixRuntime&.
};

} // namespace app::handlers::feature_matrix
