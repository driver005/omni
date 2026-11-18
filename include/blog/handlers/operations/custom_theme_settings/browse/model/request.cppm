export module blog_operations:custom_theme_settings_browse_model;

import std;

export namespace blog::handlers::operations::custom_theme_settings::browse::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
