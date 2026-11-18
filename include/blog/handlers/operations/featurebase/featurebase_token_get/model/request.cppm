export module blog_operations:featurebase_featurebase_token_get_model;

import std;

export namespace blog::handlers::operations::featurebase::featurebase_token_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
