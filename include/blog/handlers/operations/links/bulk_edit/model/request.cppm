export module blog_operations:links_bulk_edit_model;

import std;

export namespace blog::handlers::operations::links::bulk_edit::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
