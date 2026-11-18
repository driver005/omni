export module blog_operations:offers_read_by_id_model;

import std;

export namespace blog::handlers::operations::offers::read_by_id::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
