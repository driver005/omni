export module blog_operations:authentication_authentication_invitation_post_model;

import std;

export namespace blog::handlers::operations::authentication::authentication_invitation_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
