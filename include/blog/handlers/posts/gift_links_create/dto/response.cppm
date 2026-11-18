export module blog_handlers:posts_gift_links_create_dto;

import std;

export namespace blog::handlers::posts::gift_links_create::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
