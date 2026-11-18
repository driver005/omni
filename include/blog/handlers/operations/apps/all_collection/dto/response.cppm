export module blog_operations:apps_all_collection_dto;

import std;

export namespace blog::handlers::operations::apps::all_collection::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
