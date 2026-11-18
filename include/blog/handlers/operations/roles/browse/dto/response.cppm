export module blog_operations:roles_browse_dto;

import std;

export namespace blog::handlers::operations::roles::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
