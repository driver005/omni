export module blog_operations:images_images_upload_post_model;

import std;

export namespace blog::handlers::operations::images::images_upload_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
