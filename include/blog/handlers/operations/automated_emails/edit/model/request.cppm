export module blog_operations:automated_emails_edit_model;

import std;

export namespace blog::handlers::operations::automated_emails::edit::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
