export module blog_operations:notifications_destroy_model;

import std;

export namespace blog::handlers::operations::notifications::destroy::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
