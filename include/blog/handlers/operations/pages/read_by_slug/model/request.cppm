export module blog_operations:pages_read_by_slug_model;

import std;

export namespace blog::handlers::operations::pages::read_by_slug::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
