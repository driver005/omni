export module blog_operations:invites_destroy_model;

import std;

export namespace blog::handlers::operations::invites::destroy::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
