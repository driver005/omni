export module blog_operations:automations_automations_poll_put_model;

import std;

export namespace blog::handlers::operations::automations::automations_poll_put::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
