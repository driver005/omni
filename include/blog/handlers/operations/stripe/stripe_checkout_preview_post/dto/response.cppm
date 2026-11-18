export module blog_operations:stripe_stripe_checkout_preview_post_dto;

import std;

export namespace blog::handlers::operations::stripe::stripe_checkout_preview_post::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
