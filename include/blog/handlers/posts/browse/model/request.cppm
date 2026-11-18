export module blog_handlers:posts_browse_model;

import std;

export namespace blog::handlers::posts::browse::model {

struct Request
{
    std::string path{};
    std::string body{};
    std::string id{};
};

} // namespace
