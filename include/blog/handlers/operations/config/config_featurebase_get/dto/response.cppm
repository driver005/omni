export module blog_operations:config_config_featurebase_get_dto;

import std;

export namespace blog::handlers::operations::config::config_featurebase_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
