export module blog_operations:members_edit_model;

import std;

export namespace blog::handlers::operations::members::edit::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
