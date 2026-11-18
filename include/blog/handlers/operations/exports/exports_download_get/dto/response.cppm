export module blog_operations:exports_exports_download_get_dto;

import std;

export namespace blog::handlers::operations::exports::exports_download_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
