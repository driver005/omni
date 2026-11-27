export module app_routing:feature_matrix_runtime_fetch_feature_matrix;

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
import app_auth;
import app_http;
import app_connector;
import app_metrics;
import app_store;
import app_services;
import core_behavior;

import :feature_matrix_result;
import :feature_matrix_runtime_common;

export namespace app::handlers::feature_matrix {

/** @brief Port of upstream `generate_feature_matrix` (routes/feature_matrix.rs:42-73). */
class FetchFeatureMatrixRuntime
{
public:
    explicit FetchFeatureMatrixRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::feature_matrix::FeatureMatrixListResponse>
    fetch_feature_matrix(
        const ::app::dto::feature_matrix::FeatureMatrixRequest& request,
        const ::core::behavior::CallContext&
    ) const
    {
        // `#[instrument(skip_all)]` + Flow::FeatureMatrix (routes/feature_matrix.rs:19,25) and
        // api::server_wrap(..., &auth::NoAuth, LockAction::NotApplicable) (:30-38). The NO_AUTH
        // rule is a pass-through; no lock is acquired because the request is a read-only GET.
        // Auth runs in the handler; these metrics ran there too, ahead of body validation.
        m_context.metrics().record_flow("feature_matrix.fetch_feature_matrix");
        m_context.metrics().counter("feature_matrix_fetch").increment();

        // `json_payload.map(into_inner).unwrap_or_else(|| FeatureMatrixRequest { connectors: None
        // })` (routes/feature_matrix.rs:26-28). web::Json deserializes `Vec<Connector>` as
        // snake_case strings; an unknown enum string is a 400 at the web layer, so reject it here
        // too.
        std::vector<std::string> requested{};
        if (request.get_connectors()) {
            for (const auto& connector_name: *request.get_connectors()) {
                if (!m_support.connector_iter_contains(connector_name)) {
                    return std::unexpected{core::error::AppErrorBuilder{}(
                        core::error::AppErrorKind::INVALID_JSON,
                        "invalid connector name"
                    )};
                }
                requested.push_back(connector_name);
            }
        }

        // `req.connectors.unwrap_or_else(|| Connector::iter().collect())` (:46-48). The registry
        // below is the exact strum EnumIter wire-name set from common_enums/connector_enums.rs:32.
        const std::vector<std::string> connector_list =
            requested.empty() ? catalog_support::kConnectorIter : requested;

        // `connector_list.into_iter().filter_map(|connector_name| convert_connector(...)
        // .inspect_err(warn).ok().and_then(|c| build_connector_feature_details(..., c, ...)))`
        // (:50-65).
        std::vector<::app::dto::feature_matrix::ConnectorFeatureMatrixResponse>
            feature_matrix_response{};
        feature_matrix_response.reserve(connector_list.size());
        for (const auto& connector_name: connector_list) {
            if (!m_support.convert_connector(connector_name)) {
                continue;
            }
            auto built = m_support.build_connector_feature_details(connector_name);
            if (built) {
                feature_matrix_response.push_back(std::move(*built));
            }
        }

        // `Ok(ApplicationResponse::Json(FeatureMatrixListResponse { connector_count, connectors
        // }))`
        // (:67-72). The typed DTO is carried to the handler, which serializes it with
        // Ser::serialize (no hand-built JSON text).
        ::app::dto::feature_matrix::FeatureMatrixListResponse response;
        response.set_connector_count(feature_matrix_response.size());
        response.set_connectors(std::move(feature_matrix_response));

        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    FeatureMatrixRuntimeSupport m_support;
};

} // namespace app::handlers::feature_matrix
