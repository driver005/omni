export module blog_operations:email_previews_post_detail_model;

import std;

export namespace blog::handlers::operations::email_previews::post_detail::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
