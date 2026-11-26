export module app_routing:feature_matrix_input;

import std;

export namespace app::handlers::feature_matrix {

// The only request data this GET route consumes is its optional JSON body: an
// object with an optional "connectors" array of snake_case connector names.
// Body decoding stays in the runtime so the auth + metrics side effects recorded
// by the original handler still run before any body validation error is produced.
struct FetchFeatureMatrixInput
{
    std::string body{};
};

} // namespace app::handlers::feature_matrix
