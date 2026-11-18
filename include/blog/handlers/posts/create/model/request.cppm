export module blog_handlers:posts_create_model;

import std;

export namespace blog::handlers::posts::create::model {

struct Request
{
    std::string path{};
    std::string body{};
    std::string id{};
};

} // namespace
