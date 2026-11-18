export module blog_operations:members_metafields_edit_model;

import std;

export namespace blog::handlers::operations::members::metafields_edit::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
