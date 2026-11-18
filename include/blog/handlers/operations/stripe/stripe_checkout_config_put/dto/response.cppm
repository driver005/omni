export module blog_operations:stripe_stripe_checkout_config_put_dto;

import std;

export namespace blog::handlers::operations::stripe::stripe_checkout_config_put::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
