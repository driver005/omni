export module blog_operations:authors_read_by_slug_model;

import std;

export namespace blog::handlers::operations::authors::read_by_slug::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
