export module blog_operations:users_read_by_email_model;

import std;

export namespace blog::handlers::operations::users::read_by_email::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
