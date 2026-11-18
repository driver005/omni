export module blog_operations:settings_settings_verifications_put_model;

import std;

export namespace blog::handlers::operations::settings::settings_verifications_put::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
