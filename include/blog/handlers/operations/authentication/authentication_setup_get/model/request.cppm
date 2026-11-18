export module blog_operations:authentication_authentication_setup_get_model;

import std;

export namespace blog::handlers::operations::authentication::authentication_setup_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
