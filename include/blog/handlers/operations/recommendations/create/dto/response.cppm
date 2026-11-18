export module blog_operations:recommendations_create_dto;

import std;

export namespace blog::handlers::operations::recommendations::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
