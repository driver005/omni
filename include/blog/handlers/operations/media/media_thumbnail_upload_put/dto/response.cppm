export module blog_operations:media_media_thumbnail_upload_put_dto;

import std;

export namespace blog::handlers::operations::media::media_thumbnail_upload_put::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
