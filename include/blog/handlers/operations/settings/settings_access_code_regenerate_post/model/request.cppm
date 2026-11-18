export module blog_operations:settings_settings_access_code_regenerate_post_model;

import std;

export namespace blog::handlers::operations::settings::settings_access_code_regenerate_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
