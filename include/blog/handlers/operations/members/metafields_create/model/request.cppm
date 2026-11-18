export module blog_operations:members_metafields_create_model;

import std;

export namespace blog::handlers::operations::members::metafields_create::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
