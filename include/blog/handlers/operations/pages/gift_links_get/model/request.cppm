export module blog_operations:pages_gift_links_get_model;

import std;

export namespace blog::handlers::operations::pages::gift_links_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
