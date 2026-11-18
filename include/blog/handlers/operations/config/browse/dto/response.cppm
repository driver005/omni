export module blog_operations:config_browse_dto;

import std;

export namespace blog::handlers::operations::config::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
