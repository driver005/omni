export module app_routing:feature_matrix_runtime;

import std;
import app_dto;
import core_error;
import app_context;
import core_behavior;

import :feature_matrix_result;
import :feature_matrix_runtime_common;
import :feature_matrix_runtime_fetch_feature_matrix;

export namespace app::handlers::feature_matrix {

class FeatureMatrixRuntime
{
public:
    explicit FeatureMatrixRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_fetch_feature_matrix{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::feature_matrix::FeatureMatrixListResponse>
    fetch_feature_matrix(
        const ::app::dto::feature_matrix::FeatureMatrixRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_fetch_feature_matrix.fetch_feature_matrix(request, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    FetchFeatureMatrixRuntime m_fetch_feature_matrix;
};

} // namespace app::handlers::feature_matrix
