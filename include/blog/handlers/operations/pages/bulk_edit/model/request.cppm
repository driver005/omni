export module blog_operations:pages_bulk_edit_model;

import std;

export namespace blog::handlers::operations::pages::bulk_edit::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
