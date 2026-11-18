export module blog_operations:comments_comment_likes_model;

import std;

export namespace blog::handlers::operations::comments::comment_likes::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
