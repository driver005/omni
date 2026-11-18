export module blog_handlers_posts_browse:model_query_input;

import std;
import interfaces;

export namespace blog::handlers::posts::browse {

/** @brief Parsed query for GET /posts — plain model, built from IRequest. */
struct QueryInput
{
    std::size_t page{1};
    std::size_t limit{15};
    std::string fields{};
    std::string filter{};
    std::string include{};

    [[nodiscard]] static QueryInput from_request(interfaces::io::IRequest& request)
    {
        QueryInput input;
        const auto raw = std::string{request.get_path()};
        const auto q = raw.find('?');
        if (q == std::string::npos) return input;
        for (auto pair : std::string_view{raw}.substr(q + 1) | std::views::split('&')) {
            const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
            const auto eq = text.find('=');
            if (eq == std::string::npos) continue;
            const auto key = text.substr(0, eq);
            const auto value = text.substr(eq + 1);
            if (key == "page") {
                try { input.page = std::max<std::size_t>(1, std::stoul(value)); } catch (...) {}
            } else if (key == "limit") {
                try { input.limit = std::max<std::size_t>(1, std::stoul(value)); } catch (...) {}
            } else if (key == "fields") {
                input.fields = value;
            } else if (key == "filter") {
                input.filter = value;
            } else if (key == "include") {
                input.include = value;
            }
        }
        return input;
    }
};

} // namespace blog::handlers::posts::browse
