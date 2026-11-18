export module blog_operations:comments_get_detail_model;

import std;

export namespace blog::handlers::operations::comments::get_detail::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
