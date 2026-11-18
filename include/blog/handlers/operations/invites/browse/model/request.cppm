export module blog_operations:invites_browse_model;

import std;

export namespace blog::handlers::operations::invites::browse::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
