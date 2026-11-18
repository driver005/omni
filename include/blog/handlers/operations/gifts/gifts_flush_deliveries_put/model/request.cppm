export module blog_operations:gifts_gifts_flush_deliveries_put_model;

import std;

export namespace blog::handlers::operations::gifts::gifts_flush_deliveries_put::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
