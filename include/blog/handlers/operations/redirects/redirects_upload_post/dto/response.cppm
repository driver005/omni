export module blog_operations:redirects_redirects_upload_post_dto;

import std;

export namespace blog::handlers::operations::redirects::redirects_upload_post::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
