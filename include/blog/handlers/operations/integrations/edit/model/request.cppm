export module blog_operations:integrations_edit_model;

import std;

export namespace blog::handlers::operations::integrations::edit::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
