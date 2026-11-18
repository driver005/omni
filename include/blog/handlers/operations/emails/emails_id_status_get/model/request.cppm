export module blog_operations:emails_emails_id_status_get_model;

import std;

export namespace blog::handlers::operations::emails::emails_id_status_get::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
