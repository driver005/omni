export module blog_operations:recommendations_recommendations_check_post_model;

import std;

export namespace blog::handlers::operations::recommendations::recommendations_check_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
