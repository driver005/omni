export module app_handlers_feature_matrix:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_fetch_feature_matrix;

export namespace app::handlers::feature_matrix {

class FeatureMatrixRuntime
{
public:
    explicit FeatureMatrixRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_fetch_feature_matrix{context}
    {
    }

    [[nodiscard]] std::expected<FeatureMatrixFetchResult, core::error::AppError>
    fetch_feature_matrix(const FetchFeatureMatrixInput& input) const
    {
        return m_fetch_feature_matrix.fetch_feature_matrix(input);
    }

private:
    app::context::AppContext& m_context;
    FetchFeatureMatrixRuntime m_fetch_feature_matrix;
};

} // namespace app::handlers::feature_matrix
