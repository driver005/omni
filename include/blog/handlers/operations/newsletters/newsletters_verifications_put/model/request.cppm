export module blog_operations:newsletters_newsletters_verifications_put_model;

import std;

export namespace blog::handlers::operations::newsletters::newsletters_verifications_put::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
