module;
#include <rfl.hpp>
export module blog_handlers_posts_browse:dto_browse_response;

import std;
import core_serde;
import :dto_post;

export namespace blog::handlers::posts::browse::dto {

struct PaginationMeta
{
    int limit{15};
    int page{1};
    int pages{1};
    rfl::Generic next{};
    rfl::Generic prev{};
    int total{0};
};

struct BrowseResponse
{
    std::vector<PostDto> posts{};
    PaginationMeta meta{};
};

} // namespace blog::handlers::posts::browse::dto
