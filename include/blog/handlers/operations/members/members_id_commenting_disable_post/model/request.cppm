export module blog_operations:members_members_id_commenting_disable_post_model;

import std;

export namespace blog::handlers::operations::members::members_id_commenting_disable_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
