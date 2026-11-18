export module blog_operations:upgrades_browse_dto;

import std;

export namespace blog::handlers::operations::upgrades::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
