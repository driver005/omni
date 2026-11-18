export module blog_operations:stripe_stripe_checkout_config_get_dto;

import std;

export namespace blog::handlers::operations::stripe::stripe_checkout_config_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
