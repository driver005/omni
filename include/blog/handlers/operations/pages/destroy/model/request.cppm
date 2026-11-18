export module blog_operations:pages_destroy_model;

import std;

export namespace blog::handlers::operations::pages::destroy::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
