export module blog_operations:automated_emails_automated_emails_design_get_model;

import std;

export namespace blog::handlers::operations::automated_emails::automated_emails_design_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
