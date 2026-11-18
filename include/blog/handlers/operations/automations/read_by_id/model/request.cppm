export module blog_operations:automations_read_by_id_model;

import std;

export namespace blog::handlers::operations::automations::read_by_id::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
