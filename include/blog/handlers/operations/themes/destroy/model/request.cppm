export module blog_operations:themes_destroy_model;

import std;

export namespace blog::handlers::operations::themes::destroy::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
