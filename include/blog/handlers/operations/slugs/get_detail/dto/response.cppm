export module blog_operations:slugs_get_detail_dto;

import std;

export namespace blog::handlers::operations::slugs::get_detail::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
