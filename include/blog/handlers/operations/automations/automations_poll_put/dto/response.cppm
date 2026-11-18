export module blog_operations:automations_automations_poll_put_dto;

import std;

export namespace blog::handlers::operations::automations::automations_poll_put::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
