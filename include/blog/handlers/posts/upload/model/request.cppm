export module blog_handlers:posts_upload_model;

import std;

export namespace blog::handlers::posts::upload::model {

struct Request
{
    std::string path{};
    std::string body{};
    std::string id{};
};

} // namespace
