export module blog_operations:themes_themes_name_activate_put_model;

import std;

export namespace blog::handlers::operations::themes::themes_name_activate_put::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
