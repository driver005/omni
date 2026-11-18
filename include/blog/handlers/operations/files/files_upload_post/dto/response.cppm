export module blog_operations:files_files_upload_post_dto;

import std;

export namespace blog::handlers::operations::files::files_upload_post::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
