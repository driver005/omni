export module blog_operations:media_media_upload_post_dto;

import std;

export namespace blog::handlers::operations::media::media_upload_post::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
