export module blog_operations:settings_settings_stripe_connect_delete_dto;

import std;

export namespace blog::handlers::operations::settings::settings_stripe_connect_delete::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
