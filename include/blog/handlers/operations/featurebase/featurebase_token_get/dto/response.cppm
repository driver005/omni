export module blog_operations:featurebase_featurebase_token_get_dto;

import std;

export namespace blog::handlers::operations::featurebase::featurebase_token_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
