export module blog_operations:media_media_thumbnail_upload_put_model;

import std;

export namespace blog::handlers::operations::media::media_thumbnail_upload_put::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
