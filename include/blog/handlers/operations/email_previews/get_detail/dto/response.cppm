export module blog_operations:email_previews_get_detail_dto;

import std;

export namespace blog::handlers::operations::email_previews::get_detail::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
