export module blog_operations:automations_create_model;

import std;

export namespace blog::handlers::operations::automations::create::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
