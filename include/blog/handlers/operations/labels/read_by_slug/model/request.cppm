export module blog_operations:labels_read_by_slug_model;

import std;

export namespace blog::handlers::operations::labels::read_by_slug::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
