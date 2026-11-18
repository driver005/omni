export module blog_operations:email_previews_post_detail_dto;

import std;

export namespace blog::handlers::operations::email_previews::post_detail::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
