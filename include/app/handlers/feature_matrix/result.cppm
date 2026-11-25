export module app_handlers_feature_matrix:result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::feature_matrix {

// The wire response is the typed FeatureMatrixListResponse DTO (connector_count
// plus connectors). Deriving from the exact DTO and reusing its FieldDesc tuple in
// the Serializable specialization below keeps field names, order and optional
// handling intact when the handler serializes the result.
class FeatureMatrixFetchResult : public app::dto::feature_matrix::FeatureMatrixListResponse
{
public:
    FeatureMatrixFetchResult() = default;

    explicit FeatureMatrixFetchResult(
        app::dto::feature_matrix::FeatureMatrixListResponse value
    ) noexcept :
        FeatureMatrixListResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::feature_matrix

template<>
struct serde::Serializable<app::handlers::feature_matrix::FeatureMatrixFetchResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::feature_matrix::FeatureMatrixListResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "feature_matrix_fetch_result";
    }
};
