export module blog_operations:redirects_redirects_download_get_dto;

import std;

export namespace blog::handlers::operations::redirects::redirects_download_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
