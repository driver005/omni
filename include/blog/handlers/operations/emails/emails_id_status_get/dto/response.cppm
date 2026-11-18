export module blog_operations:emails_emails_id_status_get_dto;

import std;

export namespace blog::handlers::operations::emails::emails_id_status_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
