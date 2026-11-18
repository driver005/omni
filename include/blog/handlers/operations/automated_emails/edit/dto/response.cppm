export module blog_operations:automated_emails_edit_dto;

import std;

export namespace blog::handlers::operations::automated_emails::edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
