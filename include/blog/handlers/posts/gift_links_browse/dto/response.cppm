export module blog_handlers:posts_gift_links_browse_dto;

import std;

export namespace blog::handlers::posts::gift_links_browse::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
