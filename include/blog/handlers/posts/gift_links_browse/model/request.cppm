export module blog_handlers:posts_gift_links_browse_model;

import std;

export namespace blog::handlers::posts::gift_links_browse::model {

struct Request
{
    std::string path{};
    std::string body{};
    std::string id{};
};

} // namespace
