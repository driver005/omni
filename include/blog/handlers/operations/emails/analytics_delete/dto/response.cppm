export module blog_operations:emails_analytics_delete_dto;

import std;

export namespace blog::handlers::operations::emails::analytics_delete::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
