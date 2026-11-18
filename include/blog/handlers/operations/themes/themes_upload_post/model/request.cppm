export module blog_operations:themes_themes_upload_post_model;

import std;

export namespace blog::handlers::operations::themes::themes_upload_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
