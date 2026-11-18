export module blog_operations:identities_browse_dto;

import std;

export namespace blog::handlers::operations::identities::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
