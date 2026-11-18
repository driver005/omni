export module blog_operations:emails_emails_id_retry_put_model;

import std;

export namespace blog::handlers::operations::emails::emails_id_retry_put::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
