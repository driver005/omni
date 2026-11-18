export module blog_operations:comments_edit_model;

import std;

export namespace blog::handlers::operations::comments::edit::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
