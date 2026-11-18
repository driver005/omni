export module blog_handlers:posts_bulk_edit_model;

import std;

export namespace blog::handlers::posts::bulk_edit::model {

struct Request
{
    std::string path{};
    std::string body{};
    std::string id{};
};

} // namespace
