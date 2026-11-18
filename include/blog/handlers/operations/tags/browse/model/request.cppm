export module blog_operations:tags_browse_model;

import std;

export namespace blog::handlers::operations::tags::browse::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
