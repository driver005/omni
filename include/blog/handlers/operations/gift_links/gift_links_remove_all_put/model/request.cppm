export module blog_operations:gift_links_gift_links_remove_all_put_model;

import std;

export namespace blog::handlers::operations::gift_links::gift_links_remove_all_put::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
