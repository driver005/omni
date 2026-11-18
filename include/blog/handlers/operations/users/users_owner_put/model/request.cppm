export module blog_operations:users_users_owner_put_model;

import std;

export namespace blog::handlers::operations::users::users_owner_put::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
