export module blog_operations:settings_settings_stripe_connect_delete_model;

import std;

export namespace blog::handlers::operations::settings::settings_stripe_connect_delete::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
