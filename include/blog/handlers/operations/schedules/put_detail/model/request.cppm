export module blog_operations:schedules_put_detail_model;

import std;

export namespace blog::handlers::operations::schedules::put_detail::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
