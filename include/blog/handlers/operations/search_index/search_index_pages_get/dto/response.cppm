export module blog_operations:search_index_search_index_pages_get_dto;

import std;

export namespace blog::handlers::operations::search_index::search_index_pages_get::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
