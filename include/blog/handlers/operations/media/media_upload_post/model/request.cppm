export module blog_operations:media_media_upload_post_model;

import std;

export namespace blog::handlers::operations::media::media_upload_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
