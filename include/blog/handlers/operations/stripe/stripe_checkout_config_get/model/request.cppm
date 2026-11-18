export module blog_operations:stripe_stripe_checkout_config_get_model;

import std;

export namespace blog::handlers::operations::stripe::stripe_checkout_config_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
