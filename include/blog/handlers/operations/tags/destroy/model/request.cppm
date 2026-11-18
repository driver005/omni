export module blog_operations:tags_destroy_model;

import std;

export namespace blog::handlers::operations::tags::destroy::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
