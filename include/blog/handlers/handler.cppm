module;
#include <cstdio>
export module blog_handlers:handler;

import std;
import interfaces;
import blog_context;
import blog_dto;
import blog_model;
import :mutation;
import :special;
import :stripe;

export namespace blog::api {

/** @brief Shared business handler: reads IRequest, applies mutations, writes IResponse. */
class CoreHandler
{
public:
    void handle(interfaces::io::IRequest& request,
                interfaces::io::IResponse& response,
                std::function<void()> send)
    {
        const std::string method{request.get_method()};
        std::string lower_method{method};
        std::ranges::transform(lower_method, lower_method.begin(), [](char c) {
            return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        });
        const std::string raw_path{request.get_path()};
        const auto question = raw_path.find('?');
        std::string path = question == std::string::npos
                               ? raw_path
                               : raw_path.substr(0, question);
        if (path.size() > 1 && path.back() == '/') path.pop_back();

        std::string body;
        for (const std::byte byte : request.get_body()) body += static_cast<char>(byte);

        if (path.starts_with("/stripe/")) {
            if (path == "/stripe/checkout/config" && method == "GET") {
                write(response, 200, m_stripe.config());
            } else if (path == "/stripe/checkout/config" && method == "PUT") {
                m_stripe.configure(body);
                write(response, 200, m_stripe.config());
            } else if (method == "POST") {
                write(response, 200, m_stripe.preview(body));
            } else {
                write(response, 200, m_stripe.branding());
            }
            send();
            return;
        }

        const auto special = special_result(path, lower_method, body);
        if (special) {
            write(response, special->status(), special->body());
            send();
            return;
        }

        std::vector<std::string> segments;
        for (auto part : std::string_view{path} | std::views::split('/')) {
            const std::string text{std::ranges::begin(part), std::ranges::end(part)};
            if (!text.empty()) segments.push_back(text);
        }
        if (segments.empty()) {
            write(response, 404, R"({"error":"Not Found"})");
            send();
            return;
        }
        const std::string resource = segments[0];
        std::string id = segments.size() > 1 ? segments[1] : std::string{};
        if (id == "slug" || id == "email" || id == "export" || id == "events") {
            id = segments.size() > 2 ? segments[2] : std::string{};
        }
        if (id == "bulk") {
            // keep collection bulk marker
        } else if (!id.empty() && !std::ranges::all_of(id, [](char c) {
                return std::isdigit(static_cast<unsigned char>(c));
            })) {
            id.clear();
        }
        std::map<std::string, std::string> params;
        if (!id.empty()) params["id"] = id;
        if (question != std::string::npos) {
            for (auto pair : raw_path.substr(question + 1) | std::views::split('&')) {
                const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
                const auto eq = text.find('=');
                if (eq != std::string::npos) {
                    params[text.substr(0, eq)] = text.substr(eq + 1);
                }
            }
        }

        const auto raw_token = request.get_authorization();
        static constexpr std::string_view content_resources[] = {
            "posts", "pages", "tags", "authors", "settings", "newsletters", "tiers", "offers"
        };
        const bool content = std::ranges::any_of(
            content_resources, [&](std::string_view r) { return r == resource; }
        );
        if (raw_token.empty() && !content) {
            write(response, 401, R"({"error":"Unauthorized"})");
            send();
            return;
        }

        int status = 404;
        std::string out = R"({"error":"Not Found"})";
        MutationStore mutations{m_store};
        if (method == "GET") {
            if (!id.empty() && (resource == "posts" || resource == "pages" ||
                                resource == "tags" || resource == "users" ||
                                resource == "members")) {
                const auto one = m_store.find_by_resource(resource, id);
                if (!one) {
                    write(response, 404, R"({"error":"Not Found"})");
                    send();
                    return;
                }
                status = 200;
                out = std::format(R"({{"{}":[{}]}})", resource, *one);
            } else {
                status = 200;
                out = browse_body(resource, params);
            }
        } else if (method == "POST") {
            const auto result = mutations.create(resource, body.empty() ? "{}" : body);
            status = result.status(); out = result.body();
        } else if (method == "PUT") {
            const auto result = id == "bulk" ? mutations.bulk_edit(resource, body)
                                             : mutations.edit(resource, id, body);
            status = result.status(); out = result.body();
        } else if (method == "DELETE") {
            const auto result = id.empty() ? mutations.bulk_destroy(resource, body)
                                           : mutations.destroy(resource, id);
            status = result.status(); out = result.body();
        }
        write(response, status, out);
        send();
    }

private:
    void write(interfaces::io::IResponse& response, int status, std::string_view body) const
    {
        response.set_status(static_cast<interfaces::io::types::Status>(status));
        std::vector<std::byte> out;
        for (const char chr : body) out.push_back(static_cast<std::byte>(chr));
        response.set_body(std::move(out));
    }

    [[nodiscard]] std::optional<SpecialResult> special_result(
        std::string_view path,
        std::string_view method,
        std::string_view body
    )
    {
                const auto& action = method;
        std::string id{"1"};
        std::string sub{"1"};
        std::vector<std::string> parts;
        std::size_t start = 0;
        while (start < path.size()) {
            const auto slash = path.find('/', start);
            const auto stop = slash == std::string_view::npos ? path.size() : slash;
            if (stop > start) parts.emplace_back(path.substr(start, stop - start));
            if (slash == std::string_view::npos) break;
            start = slash + 1;
        }
        for (const auto& part : parts) {
            if (!part.empty() && std::ranges::all_of(part, [](char c) { return std::isdigit(c); })) {
                if (id == "1") id = part; else sub = part;
            }
        }
        if (action == "get" && (path == "/stats/mrr" || path == "/members/stats/mrr")) {
            return m_special.stats_mrr();
        }
        if (action == "get" && (path == "/stats/member_count" || path == "/members/stats/count")) {
            return m_special.stats_member();
        }
        if (path.starts_with("/stats/") || path.ends_with("/performance-stats")) {
            return m_special.stats_empty();
        }
        if (path.starts_with("/config")) {
            return m_special.config();
        }
        if (path == "/site") {
            return m_special.site();
        }
        if (action == "get" && (path == "/roles" || path == "/roles/")) {
            return m_special.roles();
        }
        if (action == "get" && path == "/invites") {
            return m_special.invites();
        }
        if (action == "get" && path == "/notifications") {
            return m_special.notifications();
        }
        if (action == "get" && path == "/labels") return m_special.labels();
        if (action == "get" && path == "/snippets") return m_special.snippets();
        if (action == "get" && path == "/recommendations") return m_special.recommendations();
        if (action == "get" && path == "/webhooks") return m_special.webhooks();
        if (action == "get" && path.starts_with("/automated_emails")) return m_special.automated_emails();
        if (action == "get" && path == "/custom_theme_settings") return m_special.custom_theme_settings();
        if (action == "get" && path == "/integrations") return m_special.integrations();
        if (action == "get" && path == "/oembed") return m_special.oembed();
        if (action == "get" && path.starts_with("/feedback/")) return m_special.feedback();
        if (action == "get" && path.starts_with("/search-index/")) {
            const auto kind = path.substr(std::string{"/search-index/"}.size());
            return m_special.search_index(kind);
        }
        if (action == "get" && path.starts_with("/email_previews/")) {
            return m_special.email_previews();
        }
        if (action == "put" && path == "/users/password") {
            return m_special.password_change();
        }
        if (action == "get" && (path.ends_with("/replies") || path.ends_with("/likes") ||
                                path.ends_with("/reports") || path.ends_with("/dislikes"))) {
            return m_special.comment_sub();
        }
        if (action == "delete" && path == "/emails/analytics") {
            return m_special.emails_analytics_delete();
        }
        if (action == "get" && path.ends_with("/recipient-failures")) {
            return m_special.emails_failures(id);
        }
        if (action == "get" && path.ends_with("/batches")) {
            return m_special.emails_batches(id);
        }
        if (action == "get" && path.ends_with("/analytics")) {
            return m_special.emails_analytics_get(id);
        }
        if (action == "put" && path.ends_with("/analytics")) {
            return m_special.emails_analytics_put(id, body);
        }
        if (action == "post" && path.ends_with("/email_preview")) {
            return m_special.automation_email_preview(id, body);
        }
        if (action == "post" && path.ends_with("/email_test")) {
            return m_special.automation_email_test(id, body);
        }
        if (action == "post" && path.ends_with("/subscriptions")) {
            return m_special.member_subscription_create(id, body);
        }
        if (action == "put" && path.contains("/subscriptions/")) {
            return m_special.member_subscription_edit(id, sub, body);
        }
        if (action == "get" && path.ends_with("/signin_urls")) {
            return m_special.member_signin_urls(id);
        }
        return std::nullopt;
    }

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

    [[nodiscard]] std::string pagination_json(
        std::size_t total,
        std::size_t page,
        std::size_t limit
    ) const
    {
        blog::dto::PaginationMetaDto meta;
        meta.limit = static_cast<int>(limit);
        meta.page = static_cast<int>(page);
        meta.total = static_cast<int>(total);
        meta.pages = total == 0 ? 1 : static_cast<int>((total + limit - 1) / limit);
        if (page < static_cast<std::size_t>(meta.pages)) {
            meta.next = static_cast<int>(page + 1);
        }
        if (page > 1) {
            meta.prev = static_cast<int>(page - 1);
        }
        return blog::dto::ModelJson<blog::dto::PaginationMetaDto>{}.to_json(meta);
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
            resource, body, pagination_json(m_store.total(resource), page, limit)
        );
    }

    ApiDataStore m_store{};
    SpecialEndpoints m_special{};
    StripeHandler m_stripe{};
};

} // namespace blog::api
