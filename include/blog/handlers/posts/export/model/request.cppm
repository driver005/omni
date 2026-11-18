export module blog_handlers:posts_export_model;

import std;

export namespace blog::handlers::posts::export_op::model {

struct Request
{
    std::string path{};
    std::string body{};
    std::string id{};
};

} // namespace
