export module blog_handlers:posts_handler;

import std;
import interfaces;
import blog_context;
import blog_dto;
import blog_model;
import :special;

export namespace blog::api {

/**
 * @brief Hand-written handler for the /posts resource. Owns query parsing,
 * response shaping and mutations for every posts route.
 */
class PostsRouteHandler
{
public:
    PostsRouteHandler(blog::BlogContext& context, SpecialEndpoints& special) noexcept :
        m_ctx{context},
        m_special{special}
    {
    }

    void handle(
        interfaces::io::IRequest& request,
        interfaces::io::IResponse& response,
        std::function<void()> send
    )
    {
        const auto method = std::string{request.get_method()};
        const auto raw = std::string{request.get_path()};
        const auto question = raw.find('?');
        auto path = question == std::string::npos ? raw : raw.substr(0, question);
        if (path.size() > 1 && path.back() == '/') path.pop_back();

        std::string body;
        for (const std::byte byte : request.get_body()) body += static_cast<char>(byte);

        const auto segments = split(path);
        if (segments.empty() || segments[0] != "posts") {
            write(response, 404, R"({"error":"Not Found"})", std::move(send));
            return;
        }

        if (path.contains("/gift_links")) {
            const auto result = m_special.gift_links();
            write(response, result.status, result.body, std::move(send));
            return;
        }
        if (path == "/posts/export") {
            const auto result = m_special.gift_links();
            write(response, result.status, result.body, std::move(send));
            return;
        }

        std::string id;
        if (segments.size() > 1 && segments[1] != "bulk") {
            if (segments[1] == "slug" || segments[1] == "email") {
                if (segments.size() > 2) id = segments[2];
            } else {
                id = segments[1];
            }
        }
        if (!id.empty() && !std::ranges::all_of(id, [](char c) {
                return std::isdigit(static_cast<unsigned char>(c));
            })) {
            id.clear();
        }

        const auto params = parse_query(
            question == std::string::npos ? std::string_view{} : std::string_view{raw}.substr(question + 1)
        );

        if (method == "GET") {
            browse_or_read(id, params, response, std::move(send));
            return;
        }

        MutationStore mutations{m_ctx.store()};
        if (method == "POST") {
            const auto result = mutations.create("posts", body.empty() ? "{}" : body);
            write(response, result.status(), result.body(), std::move(send));
            return;
        }
        if (method == "PUT") {
            const auto result = id == "bulk" || segments.back() == "bulk"
                                    ? mutations.bulk_edit("posts", body.empty() ? "{}" : body)
                                    : mutations.edit("posts", id, body.empty() ? "{}" : body);
            write(response, result.status(), result.body(), std::move(send));
            return;
        }
        if (method == "DELETE") {
            const auto result = id.empty() ? mutations.bulk_destroy("posts", body.empty() ? "{}" : body)
                                           : mutations.destroy("posts", id);
            write(response, result.status(), result.body(), std::move(send));
            return;
        }
        write(response, 404, R"({"error":"Not Found"})", std::move(send));
    }

private:
    static std::vector<std::string> split(std::string_view path)
    {
        std::vector<std::string> out;
        std::size_t start = 0;
        while (start < path.size()) {
            const auto slash = path.find('/', start);
            const auto stop = slash == std::string_view::npos ? path.size() : slash;
            if (stop > start) out.emplace_back(path.substr(start, stop - start));
            if (slash == std::string_view::npos) break;
            start = slash + 1;
        }
        return out;
    }

    static std::map<std::string, std::string> parse_query(std::string_view query)
    {
        std::map<std::string, std::string> out;
        for (auto pair : query | std::views::split('&')) {
            const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
            const auto eq = text.find('=');
            if (eq != std::string::npos) out[text.substr(0, eq)] = text.substr(eq + 1);
        }
        return out;
    }

    void browse_or_read(
        const std::string& id,
        const std::map<std::string, std::string>& params,
        interfaces::io::IResponse& response,
        std::function<void()> send
    )
    {
        if (!id.empty()) {
            const auto one = m_ctx.store().find_by_resource("posts", id);
            if (one) {
                write(response, 200, std::format(R"({{"posts":[{}]}})", *one), std::move(send));
                return;
            }
            const auto browse = m_ctx.store().json_for("posts", {});
            const auto open = browse.find('[');
            const auto close = browse.rfind(']');
            const auto first = open == std::string::npos || close == std::string::npos || close <= open
                                   ? std::string{"{}"}
                                   : browse.substr(open + 1, close - open - 1);
            write(response, 200, std::format(R"({{"posts":[{}]}})", first), std::move(send));
            return;
        }

        const auto body = m_ctx.store().json_for("posts", params);
        std::size_t page{1};
        std::size_t limit{15};
        if (const auto it = params.find("page"); it != params.end()) {
            try { page = std::max<std::size_t>(1, std::stoul(it->second)); } catch (...) {}
        }
        if (const auto it = params.find("limit"); it != params.end()) {
            try { limit = std::max<std::size_t>(1, std::stoul(it->second)); } catch (...) {}
        }
        blog::dto::PaginationMetaDto meta;
        const auto total = m_ctx.store().total("posts");
        meta.limit = static_cast<int>(limit);
        meta.page = static_cast<int>(page);
        meta.total = static_cast<int>(total);
        meta.pages = total == 0 ? 1 : static_cast<int>((total + limit - 1) / limit);
        if (page < static_cast<std::size_t>(meta.pages)) meta.next = static_cast<int>(page + 1);
        if (page > 1) meta.prev = static_cast<int>(page - 1);
        const auto pagination = blog::dto::ModelJson<blog::dto::PaginationMetaDto>{}.to_json(meta);
        write(response, 200, std::format(
            R"({{"posts":{},"meta":{{"pagination":{}}}}})", body, pagination
        ), std::move(send));
    }

    static void write(
        interfaces::io::IResponse& response,
        int status,
        std::string_view body,
        std::function<void()> send
    )
    {
        response.set_status(static_cast<interfaces::io::types::Status>(status));
        std::vector<std::byte> bytes;
        for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
        response.set_body(std::move(bytes));
        send();
    }

    blog::BlogContext& m_ctx;
    SpecialEndpoints& m_special;
};

} // namespace blog::api
