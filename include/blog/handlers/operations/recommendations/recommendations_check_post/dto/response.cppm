export module blog_operations:recommendations_recommendations_check_post_dto;

import std;

export namespace blog::handlers::operations::recommendations::recommendations_check_post::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
