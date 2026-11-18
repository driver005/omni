export module blog_operations:automated_emails_read_by_id_dto;

import std;

export namespace blog::handlers::operations::automated_emails::read_by_id::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
