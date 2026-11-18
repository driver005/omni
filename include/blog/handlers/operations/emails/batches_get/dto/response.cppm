export module blog_operations:emails_batches_get_dto;

import std;

export namespace blog::handlers::operations::emails::batches_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
