export module blog_operations:pages_gift_links_put_dto;

import std;

export namespace blog::handlers::operations::pages::gift_links_put::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
