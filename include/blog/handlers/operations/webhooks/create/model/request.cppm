export module blog_operations:webhooks_create_model;

import std;

export namespace blog::handlers::operations::webhooks::create::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
