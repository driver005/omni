export module blog_operations:settings_bulk_edit_dto;

import std;

export namespace blog::handlers::operations::settings::bulk_edit::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
