export module blog_operations:emails_analytics_get_dto;

import std;

export namespace blog::handlers::operations::emails::analytics_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
