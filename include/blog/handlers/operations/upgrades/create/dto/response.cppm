export module blog_operations:upgrades_create_dto;

import std;

export namespace blog::handlers::operations::upgrades::create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
