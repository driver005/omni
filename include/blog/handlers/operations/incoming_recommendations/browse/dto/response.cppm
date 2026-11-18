export module blog_operations:incoming_recommendations_browse_dto;

import std;

export namespace blog::handlers::operations::incoming_recommendations::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
