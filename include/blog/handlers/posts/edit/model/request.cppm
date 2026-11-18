export module blog_handlers:posts_edit_model;

import std;

export namespace blog::handlers::posts::edit::model {

struct Request
{
    std::string path{};
    std::string body{};
    std::string id{};
};

} // namespace
