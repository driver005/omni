export module blog_handlers_posts_browse:step_paginate;

import std;
import core_workflow;
import :dto_browse_response;

export namespace blog::handlers::posts::browse::workflow {

/** @brief Step: wrap the posts array fragment in the Ghost browse envelope. */
class PaginateStep
{
public:
    using Input = std::map<std::string, std::string>;
    using Output = std::string;
    static constexpr std::string_view name{"paginate"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& values,
        core::workflow::WorkflowContext&
    ) const
    {
        const auto posts = values.contains("posts") ? values.at("posts") : std::string{"[]"};
        const auto page = values.contains("page") ? values.at("page") : std::string{"1"};
        const auto limit = values.contains("limit") ? values.at("limit") : std::string{"15"};
        const auto total = values.contains("total") ? values.at("total") : std::string{"0"};
        std::string out = "{\"posts\":" + posts +
            ",\"meta\":{\"pagination\":{\"limit\":" + limit +
            ",\"page\":" + page +
            ",\"pages\":1,\"prev\":null,\"next\":null,\"total\":" + total + "}}}";
        return core::workflow::StepResponse<Output>{std::move(out)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
};

} // namespace blog::handlers::posts::browse::workflow
