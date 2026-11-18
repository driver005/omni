export module blog_operations:users_users_id_token_get_model;

import std;

export namespace blog::handlers::operations::users::users_id_token_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
