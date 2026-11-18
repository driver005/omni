export module blog_operations:members_members_id_suppression_delete_model;

import std;

export namespace blog::handlers::operations::members::members_id_suppression_delete::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
