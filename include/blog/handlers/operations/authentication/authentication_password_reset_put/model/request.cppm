export module blog_operations:authentication_authentication_password_reset_put_model;

import std;

export namespace blog::handlers::operations::authentication::authentication_password_reset_put::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
