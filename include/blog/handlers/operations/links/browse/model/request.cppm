export module blog_operations:links_browse_model;

import std;

export namespace blog::handlers::operations::links::browse::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
