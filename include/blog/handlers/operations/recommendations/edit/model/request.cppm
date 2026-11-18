export module blog_operations:recommendations_edit_model;

import std;

export namespace blog::handlers::operations::recommendations::edit::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
