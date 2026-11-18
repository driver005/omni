export module blog_handlers_posts_browse:flow_browse;

import std;
import core_workflow;
import :step_parse_query;
import :step_query_posts;
import :step_paginate;

export namespace blog::handlers::posts::browse::workflow {

/** @brief Workflow definition for GET /posts. */
class BrowsePostsFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("posts_browse")
            .step<ParseQueryStep>()
            .step<QueryPostsStep>([](core::workflow::WorkflowContext& context) {
                return context.step_output_as<std::map<std::string, std::string>>("parse_query");
            })
            .step<PaginateStep>([](core::workflow::WorkflowContext& context) {
                auto values = context.step_output_as<std::map<std::string, std::string>>("parse_query");
                values["posts"] = context.step_output_as<std::string>("query_posts");
                return values;
            })
            .build();
    }
};

} // namespace blog::handlers::posts::browse::workflow
