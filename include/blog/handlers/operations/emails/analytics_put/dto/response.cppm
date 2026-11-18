export module blog_operations:emails_analytics_put_dto;

import std;

export namespace blog::handlers::operations::emails::analytics_put::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
