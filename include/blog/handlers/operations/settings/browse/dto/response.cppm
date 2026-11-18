export module blog_operations:settings_browse_dto;

import std;

export namespace blog::handlers::operations::settings::browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
