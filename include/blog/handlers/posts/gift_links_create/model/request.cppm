export module blog_handlers:posts_gift_links_create_model;

import std;

export namespace blog::handlers::posts::gift_links_create::model {

struct Request
{
    std::string path{};
    std::string body{};
    std::string id{};
};

} // namespace
