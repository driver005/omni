export module blog_handlers:posts_bulk_destroy_model;

import std;

export namespace blog::handlers::posts::bulk_destroy::model {

struct Request
{
    std::string path{};
    std::string body{};
    std::string id{};
};

} // namespace
