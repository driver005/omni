export module blog_operations:tags_edit_model;

import std;

export namespace blog::handlers::operations::tags::edit::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
