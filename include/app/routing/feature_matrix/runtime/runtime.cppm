export module app_routing:feature_matrix_runtime;

import std;
import app_dto;
import core_error;
import app_context;

import :feature_matrix_input;
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

    [[nodiscard]] std::
        expected<::app::dto::feature_matrix::FeatureMatrixListResponse, core::error::AppError>
        fetch_feature_matrix(const FetchFeatureMatrixInput& input) const
    {
        return m_fetch_feature_matrix.fetch_feature_matrix(input);
    }

private:
    ::app::context::AppContext& m_context;
    FetchFeatureMatrixRuntime m_fetch_feature_matrix;
};

} // namespace app::handlers::feature_matrix
