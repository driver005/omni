export module blog_api:dispatcher;

import std;
import :request;
import :response;
import :route_table;
import :datastore;
import :pagination;
import :stripe;
import :mutation;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

/**
 * @brief Real Ghost API dispatcher: matches method+path against the full route
 * table, extracts params, checks admin auth, and returns a resource-shaped body.
 */
class ApiDispatcher
{
public:
    ApiDispatcher() :
        m_table{}
    {
    }

    [[nodiscard]] ApiResponse dispatch(const ApiRequest& request)
    {
        // Ghost exposes collection DELETE for bulk removal on several resources;
        // handle it even when the route table only lists the /:id variant.
        const auto qpos = request.path().find('?');
        const std::string clean = qpos == std::string::npos
                                      ? request.path()
                                      : request.path().substr(0, qpos);
        if (request.method() == "DELETE" && clean.size() > 1 && clean.find('/', 1) == std::string::npos) {
            const std::string resource = clean.substr(1);
            if (!mapped_resource(resource)) {
                MutationStore mutations{m_store};
                const auto result = mutations.bulk_destroy(resource, request.body());
                return ApiResponse{result.status(), result.body(), !result.ok()};
            }
        }
        const auto question = request.path().find('?');
        const std::string_view path = question == std::string::npos
                                          ? std::string_view{request.path()}
                                          : std::string_view{request.path()}.substr(0, question);
        const ApiRoute* best = nullptr;
        std::map<std::string, std::string> best_params;
        for (const ApiRoute& route: m_table.routes()) {
            if (route.method() != request.method()) {
                continue;
            }
            std::map<std::string, std::string> params = query_params(request.path());
            if (!route.match(path, params)) {
                continue;
            }
            if (best == nullptr || params.size() < best_params.size()) {
                best = &route;
                best_params = params;
            }
        }
        if (best != nullptr) {
            const ApiRoute& route = *best;
            auto params = best_params;
            {
            if (route.admin() && request.token().empty()) {
                return ApiResponse{401, R"({"error":"Unauthorized"})", true};
            }
            if (route.resource() == "stripe") {
                if (route.path() == "/stripe/checkout/config" && route.action() == "get") {
                    return ApiResponse{200, m_stripe.config()};
                }
                if (route.path() == "/stripe/checkout/config" && route.action() == "put") {
                    m_stripe.configure(request.body());
                    return ApiResponse{200, m_stripe.config()};
                }
                if (route.action() == "post") {
                    return ApiResponse{200, m_stripe.preview(request.body())};
                }
                return ApiResponse{200, m_stripe.branding()};
            }
            if (route.admin() && route.action() == "get" && !params.contains("id") &&
                !mapped_resource(route.resource())) {
                return ApiResponse{200, std::format(
                    R"({{"{}":{},"meta":{{"pagination":{}}}}})",
                    route.resource(),
                    m_store.resources().list(route.resource()),
                    PaginationMeta{}.json(
                        m_store.resources().count(route.resource()), 1, 15
                    )
                )};
            }
            if (route.admin() && (route.action() == "post" || route.action() == "put" ||
                                  route.action() == "delete")) {
                return mutate(route, request, params);
            }
            if (!route.admin() && route.action() == "get" && !params.contains("id") &&
                (route.resource() == "posts" || route.resource() == "pages" ||
                 route.resource() == "tags" || route.resource() == "authors" ||
                 route.resource() == "settings" || route.resource() == "newsletters" ||
                 route.resource() == "tiers")) {
                return ApiResponse{200, browse_body(route.resource(), params)};
            }
            if (route.resource() == "posts" && route.action() == "get" && params.contains("id")) {
                const auto post_json = posts_body(params);
                if (!post_json) {
                    return ApiResponse{404, R"({"error":"Not Found"})", true};
                }
                return ApiResponse{200, *post_json};
            }
            return ApiResponse{200, body(route, params)};
            }
        }
        return ApiResponse{404, R"({"error":"Not Found"})", true};
    }

    [[nodiscard]] const RouteTable& table() const noexcept
    {
        return m_table;
    }

private:
    [[nodiscard]] std::map<std::string, std::string> query_params(std::string_view path) const
    {
        std::map<std::string, std::string> result;
        const auto question = path.find('?');
        if (question == std::string_view::npos) {
            return result;
        }
        auto rest = path.substr(question + 1);
        while (!rest.empty()) {
            const auto amp = rest.find('&');
            const auto pair = rest.substr(0, amp);
            const auto eq = pair.find('=');
            if (eq != std::string_view::npos) {
                result[std::string{pair.substr(0, eq)}] = std::string{pair.substr(eq + 1)};
            }
            if (amp == std::string_view::npos) {
                break;
            }
            rest = rest.substr(amp + 1);
        }
        return result;
    }

    [[nodiscard]] bool mapped_resource(std::string_view resource) const
    {
        return resource == "posts" || resource == "pages" || resource == "tags" ||
               resource == "users" || resource == "members" ||
               resource == "newsletters" || resource == "tiers";
    }

    [[nodiscard]] std::string browse_body(
        std::string_view resource,
        const std::map<std::string, std::string>& params
    ) const
    {
        std::size_t page = 1;
        std::size_t limit = 15;
        if (const auto it = params.find("page"); it != params.end()) {
            try { page = std::max<std::size_t>(1, std::stoul(it->second)); } catch (...) {}
        }
        if (const auto it = params.find("limit"); it != params.end()) {
            try { limit = std::max<std::size_t>(1, std::stoul(it->second)); } catch (...) {}
        }
        const auto body = m_store.json_for(resource, params);
        if (resource == "settings") {
            return std::format(R"({{"settings":{},"meta":{{}}}})", body);
        }
        return std::format(
            R"({{"{}":{},"meta":{{"pagination":{}}}}})",
            resource, body, PaginationMeta{}.json(m_store.total(resource), page, limit)
        );
    }

    [[nodiscard]] std::string body(
        const ApiRoute& route,
        const std::map<std::string, std::string>& params
    ) const
    {
        std::string out = R"({"resource":")" + route.resource() + R"(","action":")" +
                          route.action() + R"(","params":{)";
        bool first = true;
        for (const auto& [key, value]: params) {
            out += (first ? "" : ",") + std::format(R"("{}":"{}")", key, value);
            first = false;
        }
        out += "}}";
        return out;
    }

    [[nodiscard]] std::optional<std::string> posts_body(
        const std::map<std::string, std::string>& params
    ) const
    {
        auto it = params.find("id");
        if (it != params.end()) {
            const auto post = m_store.post(it->second);
            if (!post) {
                return std::nullopt;
            }
            return std::format(
                R"({{"id":{},"uuid":"{}","title":"{}","slug":"{}","status":"{}"}})",
                post->id, post->uuid, post->title, post->slug, post->status
            );
        }
        std::string out = "[";
        bool first = true;
        for (const auto& post: m_store.posts()) {
            out += (first ? "" : ",") + std::format(
                R"({{"id":{},"uuid":"{}","title":"{}","slug":"{}","status":"{}"}})",
                post.id, post.uuid, post.title, post.slug, post.status
            );
            first = false;
        }
        out += "]";
        return out;
    }

    [[nodiscard]] ApiResponse mutate(
        const ApiRoute& route,
        const ApiRequest& request,
        const std::map<std::string, std::string>& params
    )
    {
        MutationStore mutations{m_store};
        const std::string& resource = route.resource();
        const bool bulk = request.path().ends_with("/bulk") ||
                          (route.action() == "delete" && !params.contains("id"));
        if (bulk && route.action() == "delete") {
            const auto result = mutations.bulk_destroy(resource, request.body());
            return ApiResponse{result.status(), result.body(), !result.ok()};
        }
        if (bulk && route.action() == "put") {
            const auto result = mutations.bulk_edit(resource, request.body());
            return ApiResponse{result.status(), result.body(), !result.ok()};
        }
        if (route.action() == "post") {
            const auto result = mutations.create(resource, request.body());
            return ApiResponse{result.status(), result.body(), !result.ok()};
        }
        if (route.action() == "put") {
            auto it = params.find("id");
            const std::string id = it == params.end() ? std::string{} : it->second;
            const auto result = mutations.edit(resource, id, request.body());
            return ApiResponse{result.status(), result.body(), !result.ok()};
        }
        auto it = params.find("id");
        const std::string id = it == params.end() ? std::string{} : it->second;
        const auto result = mutations.destroy(resource, id);
        return ApiResponse{result.status(), result.body(), !result.ok()};
    }

    RouteTable m_table{};
    ApiDataStore m_store{};
    StripeHandler m_stripe{};
};

} // namespace blog::api

#ifdef CONGELADO_TEST
namespace blog::api::tests {
using namespace boost::ut;
suite<"blog_api_dispatcher"> blog_api_dispatcher_suite = []
{
    "dispatches content route without auth"_test = []
    {
        ApiDispatcher dispatcher;
        const auto response = dispatcher.dispatch(ApiRequest{"GET", "/posts"});
        expect(response.ok());
        expect(response.body().contains("posts"));
    };
    "requires auth for admin route"_test = []
    {
        ApiDispatcher dispatcher;
        const auto denied = dispatcher.dispatch(ApiRequest{"GET", "/members"});
        expect(denied.status() == 401);
        const auto allowed = dispatcher.dispatch(
            ApiRequest{"GET", "/members"}.token("admin-token")
        );
        expect(allowed.ok());
    };
    "unknown route is 404"_test = []
    {
        ApiDispatcher dispatcher;
        expect(dispatcher.dispatch(ApiRequest{"GET", "/nope"}).status() == 404);
    };
};
} // namespace blog::api::tests
#endif
