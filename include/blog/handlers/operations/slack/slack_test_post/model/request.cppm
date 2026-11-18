export module blog_operations:slack_slack_test_post_model;

import std;

export namespace blog::handlers::operations::slack::slack_test_post::model {

struct RequestData
{
    std::string id{};
    std::string sub{};
    std::string body{};
    std::string path{};
    std::string token{};
};

} // namespace
