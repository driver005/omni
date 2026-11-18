export module blog_operations:automated_emails_automated_emails_design_put_model;

import std;

export namespace blog::handlers::operations::automated_emails::automated_emails_design_put::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
