export module blog_operations:actions_browse_dto;

import std;

export namespace blog::handlers::operations::actions::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
