export module blog_operations:tinybird_tinybird_token_get_dto;

import std;

export namespace blog::handlers::operations::tinybird::tinybird_token_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
