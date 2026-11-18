export module blog_operations:comments_comment_dislikes_dto;

import std;

export namespace blog::handlers::operations::comments::comment_dislikes::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
