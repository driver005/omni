export module blog_operations:integrations_integrations_id_api_key_keyid_refresh_post_model;

import std;

export namespace blog::handlers::operations::integrations::integrations_id_api_key_keyid_refresh_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
