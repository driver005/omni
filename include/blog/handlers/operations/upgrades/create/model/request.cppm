export module blog_operations:upgrades_create_model;

import std;

export namespace blog::handlers::operations::upgrades::create::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
