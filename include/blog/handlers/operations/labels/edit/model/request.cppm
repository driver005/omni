export module blog_operations:labels_edit_model;

import std;

export namespace blog::handlers::operations::labels::edit::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
