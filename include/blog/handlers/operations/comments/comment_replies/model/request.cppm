export module blog_operations:comments_comment_replies_model;

import std;

export namespace blog::handlers::operations::comments::comment_replies::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
