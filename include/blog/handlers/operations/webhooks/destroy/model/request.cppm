export module blog_operations:webhooks_destroy_model;

import std;

export namespace blog::handlers::operations::webhooks::destroy::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
