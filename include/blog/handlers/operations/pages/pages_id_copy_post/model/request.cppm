export module blog_operations:pages_pages_id_copy_post_model;

import std;

export namespace blog::handlers::operations::pages::pages_id_copy_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
