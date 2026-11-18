export module blog_operations:recommendations_destroy_dto;

import std;

export namespace blog::handlers::operations::recommendations::destroy::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
