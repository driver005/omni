export module blog_operations:comments_comment_likes_dto;

import std;

export namespace blog::handlers::operations::comments::comment_likes::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
