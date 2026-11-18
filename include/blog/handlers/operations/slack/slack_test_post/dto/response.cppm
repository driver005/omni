export module blog_operations:slack_slack_test_post_dto;

import std;

export namespace blog::handlers::operations::slack::slack_test_post::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
