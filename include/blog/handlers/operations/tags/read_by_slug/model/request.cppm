export module blog_operations:tags_read_by_slug_model;

import std;

export namespace blog::handlers::operations::tags::read_by_slug::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
