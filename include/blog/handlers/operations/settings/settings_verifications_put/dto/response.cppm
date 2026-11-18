export module blog_operations:settings_settings_verifications_put_dto;

import std;

export namespace blog::handlers::operations::settings::settings_verifications_put::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
