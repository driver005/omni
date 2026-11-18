export module blog_operations:themes_themes_active_get_model;

import std;

export namespace blog::handlers::operations::themes::themes_active_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
