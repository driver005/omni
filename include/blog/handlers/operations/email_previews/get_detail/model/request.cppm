export module blog_operations:email_previews_get_detail_model;

import std;

export namespace blog::handlers::operations::email_previews::get_detail::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
