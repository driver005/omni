export module blog_handlers:posts_copy_model;

import std;

export namespace blog::handlers::posts::copy::model {

struct Request
{
    std::string path{};
    std::string body{};
    std::string id{};
};

} // namespace
