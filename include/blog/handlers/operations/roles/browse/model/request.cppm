export module blog_operations:roles_browse_model;

import std;

export namespace blog::handlers::operations::roles::browse::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
