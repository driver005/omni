export module blog_operations:pages_gift_links_post_model;

import std;

export namespace blog::handlers::operations::pages::gift_links_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
