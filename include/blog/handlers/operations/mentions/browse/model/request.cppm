export module blog_operations:mentions_browse_model;

import std;

export namespace blog::handlers::operations::mentions::browse::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
