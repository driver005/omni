export module blog_api:route_table;

import std;
import :route;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

/** @brief Full Ghost API route table (Content + Admin), generated from routes.js. */
class RouteTable
{
public:
    RouteTable()
    {
        m_routes.emplace_back("GET", "/posts", "posts", "get", false);
        m_routes.emplace_back("GET", "/posts/:id", "posts", "get", false);
        m_routes.emplace_back("GET", "/posts/slug/:slug", "posts", "get", false);
        m_routes.emplace_back("GET", "/pages", "pages", "get", false);
        m_routes.emplace_back("GET", "/pages/:id", "pages", "get", false);
        m_routes.emplace_back("GET", "/pages/slug/:slug", "pages", "get", false);
        m_routes.emplace_back("GET", "/authors", "authors", "get", false);
        m_routes.emplace_back("GET", "/authors/:id", "authors", "get", false);
        m_routes.emplace_back("GET", "/authors/slug/:slug", "authors", "get", false);
        m_routes.emplace_back("GET", "/tags", "tags", "get", false);
        m_routes.emplace_back("GET", "/tags/:id", "tags", "get", false);
        m_routes.emplace_back("GET", "/tags/slug/:slug", "tags", "get", false);
        m_routes.emplace_back("GET", "/settings", "settings", "get", false);
        m_routes.emplace_back("GET", "/newsletters", "newsletters", "get", false);
        m_routes.emplace_back("GET", "/tiers", "tiers", "get", false);
        m_routes.emplace_back("GET", "/offers/:id", "offers", "get", false);
        m_routes.emplace_back("GET", "/recommendations", "recommendations", "get", false);
        m_routes.emplace_back("GET", "/search-index/posts", "search-index", "get", false);
        m_routes.emplace_back("GET", "/search-index/authors", "search-index", "get", false);
        m_routes.emplace_back("GET", "/search-index/tags", "search-index", "get", false);
        m_routes.emplace_back("GET", "/site", "site", "get", true);
        m_routes.emplace_back("GET", "/config", "config", "get", true);
        m_routes.emplace_back("GET", "/config/featurebase", "config", "get", true);
        m_routes.emplace_back("GET", "/posts", "posts", "get", true);
        m_routes.emplace_back("GET", "/posts/export", "posts", "get", true);
        m_routes.emplace_back("POST", "/posts", "posts", "post", true);
        m_routes.emplace_back("DELETE", "/posts", "posts", "delete", true);
        m_routes.emplace_back("PUT", "/posts/bulk", "posts", "put", true);
        m_routes.emplace_back("POST", "/posts/upload", "posts", "post", true);
        m_routes.emplace_back("GET", "/posts/:id", "posts", "get", true);
        m_routes.emplace_back("GET", "/posts/slug/:slug", "posts", "get", true);
        m_routes.emplace_back("PUT", "/posts/:id", "posts", "put", true);
        m_routes.emplace_back("DELETE", "/posts/:id", "posts", "delete", true);
        m_routes.emplace_back("POST", "/posts/:id/copy", "posts", "post", true);
        m_routes.emplace_back("GET", "/mentions", "mentions", "get", true);
        m_routes.emplace_back("GET", "/comments", "comments", "get", true);
        m_routes.emplace_back("GET", "/comments/:id", "comments", "get", true);
        m_routes.emplace_back("GET", "/comments/:id/replies", "comments", "get", true);
        m_routes.emplace_back("GET", "/comments/:id/reports", "comments", "get", true);
        m_routes.emplace_back("GET", "/comments/:id/likes", "comments", "get", true);
        m_routes.emplace_back("GET", "/comments/:id/dislikes", "comments", "get", true);
        m_routes.emplace_back("GET", "/comments/post/:post_id", "comments", "get", true);
        m_routes.emplace_back("POST", "/comments", "comments", "post", true);
        m_routes.emplace_back("PUT", "/comments/:id", "comments", "put", true);
        m_routes.emplace_back("GET", "/pages", "pages", "get", true);
        m_routes.emplace_back("DELETE", "/pages", "pages", "delete", true);
        m_routes.emplace_back("PUT", "/pages/bulk", "pages", "put", true);
        m_routes.emplace_back("POST", "/pages", "pages", "post", true);
        m_routes.emplace_back("GET", "/pages/:id", "pages", "get", true);
        m_routes.emplace_back("GET", "/pages/slug/:slug", "pages", "get", true);
        m_routes.emplace_back("PUT", "/pages/:id", "pages", "put", true);
        m_routes.emplace_back("DELETE", "/pages/:id", "pages", "delete", true);
        m_routes.emplace_back("POST", "/pages/:id/copy", "pages", "post", true);
        m_routes.emplace_back("GET", "/posts/:id/gift_links", "posts", "get", true);
        m_routes.emplace_back("PUT", "/posts/:id/gift_links", "posts", "put", true);
        m_routes.emplace_back("POST", "/posts/:id/gift_links", "posts", "post", true);
        m_routes.emplace_back("GET", "/pages/:id/gift_links", "pages", "get", true);
        m_routes.emplace_back("PUT", "/pages/:id/gift_links", "pages", "put", true);
        m_routes.emplace_back("POST", "/pages/:id/gift_links", "pages", "post", true);
        m_routes.emplace_back("PUT", "/gift_links/remove_all", "gift_links", "put", true);
        m_routes.emplace_back("GET", "/integrations", "integrations", "get", true);
        m_routes.emplace_back("GET", "/integrations/:id", "integrations", "get", true);
        m_routes.emplace_back("POST", "/integrations", "integrations", "post", true);
        m_routes.emplace_back("POST", "/integrations/:id/api_key/:keyid/refresh", "integrations", "post", true);
        m_routes.emplace_back("PUT", "/integrations/:id", "integrations", "put", true);
        m_routes.emplace_back("DELETE", "/integrations/:id", "integrations", "delete", true);
        m_routes.emplace_back("PUT", "/schedules/:resource/:id", "schedules", "put", true);
        m_routes.emplace_back("PUT", "/gifts/flush_reminders", "gifts", "put", true);
        m_routes.emplace_back("PUT", "/gifts/flush_deliveries", "gifts", "put", true);
        m_routes.emplace_back("GET", "/settings/routes/yaml", "settings", "get", true);
        m_routes.emplace_back("POST", "/settings/routes/yaml", "settings", "post", true);
        m_routes.emplace_back("GET", "/settings", "settings", "get", true);
        m_routes.emplace_back("PUT", "/settings", "settings", "put", true);
        m_routes.emplace_back("POST", "/settings/access_code/regenerate", "settings", "post", true);
        m_routes.emplace_back("PUT", "/settings/verifications/", "settings", "put", true);
        m_routes.emplace_back("DELETE", "/settings/stripe/connect", "settings", "delete", true);
        m_routes.emplace_back("GET", "/users", "users", "get", true);
        m_routes.emplace_back("GET", "/users/:id", "users", "get", true);
        m_routes.emplace_back("GET", "/users/slug/:slug", "users", "get", true);
        m_routes.emplace_back("GET", "/users/email/:email", "users", "get", true);
        m_routes.emplace_back("GET", "/users/:id/token", "users", "get", true);
        m_routes.emplace_back("PUT", "/users/password", "users", "put", true);
        m_routes.emplace_back("PUT", "/users/owner", "users", "put", true);
        m_routes.emplace_back("PUT", "/users/:id", "users", "put", true);
        m_routes.emplace_back("PUT", "/users/:id/token", "users", "put", true);
        m_routes.emplace_back("DELETE", "/users/:id", "users", "delete", true);
        m_routes.emplace_back("GET", "/tags", "tags", "get", true);
        m_routes.emplace_back("GET", "/tags/:id", "tags", "get", true);
        m_routes.emplace_back("GET", "/tags/slug/:slug", "tags", "get", true);
        m_routes.emplace_back("POST", "/tags", "tags", "post", true);
        m_routes.emplace_back("PUT", "/tags/:id", "tags", "put", true);
        m_routes.emplace_back("DELETE", "/tags/:id", "tags", "delete", true);
        m_routes.emplace_back("GET", "/tiers", "tiers", "get", true);
        m_routes.emplace_back("POST", "/tiers", "tiers", "post", true);
        m_routes.emplace_back("GET", "/tiers/:id", "tiers", "get", true);
        m_routes.emplace_back("PUT", "/tiers/:id", "tiers", "put", true);
        m_routes.emplace_back("GET", "/stripe/checkout/config", "stripe", "get", true);
        m_routes.emplace_back("PUT", "/stripe/checkout/config", "stripe", "put", true);
        m_routes.emplace_back("POST", "/stripe/checkout/preview", "stripe", "post", true);
        m_routes.emplace_back("GET", "/stripe/checkout/branding", "stripe", "get", true);
        m_routes.emplace_back("GET", "/members", "members", "get", true);
        m_routes.emplace_back("POST", "/members", "members", "post", true);
        m_routes.emplace_back("DELETE", "/members", "members", "delete", true);
        m_routes.emplace_back("PUT", "/members/bulk", "members", "put", true);
        m_routes.emplace_back("GET", "/offers", "offers", "get", true);
        m_routes.emplace_back("POST", "/offers", "offers", "post", true);
        m_routes.emplace_back("GET", "/offers/:id", "offers", "get", true);
        m_routes.emplace_back("PUT", "/offers/:id", "offers", "put", true);
        m_routes.emplace_back("GET", "/members/stats/count", "members", "get", true);
        m_routes.emplace_back("GET", "/members/stats/mrr", "members", "get", true);
        m_routes.emplace_back("GET", "/members/events", "members", "get", true);
        m_routes.emplace_back("GET", "/members/upload", "members", "get", true);
        m_routes.emplace_back("POST", "/members/upload", "members", "post", true);
        m_routes.emplace_back("GET", "/members/stripe_connect", "members", "get", true);
        m_routes.emplace_back("ALL", "/members/metafields", "members", "all", true);
        m_routes.emplace_back("ALL", "/apps", "apps", "all", true);
        m_routes.emplace_back("GET", "/members/:id", "members", "get", true);
        m_routes.emplace_back("PUT", "/members/:id", "members", "put", true);
        m_routes.emplace_back("DELETE", "/members/:id", "members", "delete", true);
        m_routes.emplace_back("DELETE", "/members/:id/sessions", "members", "delete", true);
        m_routes.emplace_back("DELETE", "/members/:id/suppression", "members", "delete", true);
        m_routes.emplace_back("POST", "/members/:id/subscriptions/", "members", "post", true);
        m_routes.emplace_back("PUT", "/members/:id/subscriptions/:subscription_id", "members", "put", true);
        m_routes.emplace_back("GET", "/members/:id/signin_urls", "members", "get", true);
        m_routes.emplace_back("POST", "/members/:id/commenting/disable", "members", "post", true);
        m_routes.emplace_back("POST", "/members/:id/commenting/enable", "members", "post", true);
        m_routes.emplace_back("GET", "/stats/member_count", "stats", "get", true);
        m_routes.emplace_back("GET", "/stats/mrr", "stats", "get", true);
        m_routes.emplace_back("GET", "/stats/subscriptions", "stats", "get", true);
        m_routes.emplace_back("GET", "/stats/referrers", "stats", "get", true);
        m_routes.emplace_back("GET", "/stats/posts/:id/stats", "stats", "get", true);
        m_routes.emplace_back("GET", "/stats/top-posts", "stats", "get", true);
        m_routes.emplace_back("GET", "/stats/top-posts-views", "stats", "get", true);
        m_routes.emplace_back("GET", "/stats/top-content", "stats", "get", true);
        m_routes.emplace_back("GET", "/stats/newsletter-stats", "stats", "get", true);
        m_routes.emplace_back("GET", "/stats/newsletter-basic-stats", "stats", "get", true);
        m_routes.emplace_back("GET", "/stats/newsletter-click-stats", "stats", "get", true);
        m_routes.emplace_back("GET", "/stats/subscriber-count", "stats", "get", true);
        m_routes.emplace_back("GET", "/stats/posts/:id/top-referrers", "stats", "get", true);
        m_routes.emplace_back("GET", "/stats/posts/:id/growth", "stats", "get", true);
        m_routes.emplace_back("GET", "/stats/top-sources-growth", "stats", "get", true);
        m_routes.emplace_back("POST", "/stats/posts-visitor-counts", "stats", "post", true);
        m_routes.emplace_back("POST", "/stats/posts-member-counts", "stats", "post", true);
        m_routes.emplace_back("GET", "/labels", "labels", "get", true);
        m_routes.emplace_back("GET", "/labels/:id", "labels", "get", true);
        m_routes.emplace_back("GET", "/labels/slug/:slug", "labels", "get", true);
        m_routes.emplace_back("POST", "/labels", "labels", "post", true);
        m_routes.emplace_back("PUT", "/labels/:id", "labels", "put", true);
        m_routes.emplace_back("DELETE", "/labels/:id", "labels", "delete", true);
        m_routes.emplace_back("GET", "/automations", "automations", "get", true);
        m_routes.emplace_back("GET", "/automations/:automation_id/actions/:action_id/links", "automations", "get", true);
        m_routes.emplace_back("GET", "/automations/:id", "automations", "get", true);
        m_routes.emplace_back("GET", "/automations/:id/performance-stats", "automations", "get", true);
        m_routes.emplace_back("GET", "/automations/:id/runs", "automations", "get", true);
        m_routes.emplace_back("GET", "/automations/:id/runs/:run_id", "automations", "get", true);
        m_routes.emplace_back("POST", "/automations/:id/email_preview", "automations", "post", true);
        m_routes.emplace_back("POST", "/automations/:id/email_test", "automations", "post", true);
        m_routes.emplace_back("PUT", "/automations/poll", "automations", "put", true);
        m_routes.emplace_back("POST", "/automations", "automations", "post", true);
        m_routes.emplace_back("PUT", "/automations/:id", "automations", "put", true);
        m_routes.emplace_back("GET", "/automated_emails", "automated_emails", "get", true);
        m_routes.emplace_back("GET", "/automated_emails/design", "automated_emails", "get", true);
        m_routes.emplace_back("PUT", "/automated_emails/senders", "automated_emails", "put", true);
        m_routes.emplace_back("PUT", "/automated_emails/verifications", "automated_emails", "put", true);
        m_routes.emplace_back("GET", "/automated_emails/:id", "automated_emails", "get", true);
        m_routes.emplace_back("POST", "/automated_emails", "automated_emails", "post", true);
        m_routes.emplace_back("PUT", "/automated_emails/design", "automated_emails", "put", true);
        m_routes.emplace_back("PUT", "/automated_emails/:id", "automated_emails", "put", true);
        m_routes.emplace_back("POST", "/automated_emails/:id/preview", "automated_emails", "post", true);
        m_routes.emplace_back("POST", "/automated_emails/:id/test", "automated_emails", "post", true);
        m_routes.emplace_back("GET", "/roles/", "roles", "get", true);
        m_routes.emplace_back("GET", "/slugs/:type/:name", "slugs", "get", true);
        m_routes.emplace_back("GET", "/slugs/:type/:name/:id", "slugs", "get", true);
        m_routes.emplace_back("GET", "/themes/", "themes", "get", true);
        m_routes.emplace_back("GET", "/themes/:name/download", "themes", "get", true);
        m_routes.emplace_back("GET", "/themes/active", "themes", "get", true);
        m_routes.emplace_back("POST", "/themes/upload", "themes", "post", true);
        m_routes.emplace_back("POST", "/themes/install", "themes", "post", true);
        m_routes.emplace_back("PUT", "/themes/:name/activate", "themes", "put", true);
        m_routes.emplace_back("DELETE", "/themes/:name", "themes", "delete", true);
        m_routes.emplace_back("GET", "/notifications", "notifications", "get", true);
        m_routes.emplace_back("POST", "/notifications", "notifications", "post", true);
        m_routes.emplace_back("DELETE", "/notifications/:notification_id", "notifications", "delete", true);
        m_routes.emplace_back("GET", "/db", "db", "get", true);
        m_routes.emplace_back("POST", "/db", "db", "post", true);
        m_routes.emplace_back("DELETE", "/db", "db", "delete", true);
        m_routes.emplace_back("POST", "/db/backup", "db", "post", true);
        m_routes.emplace_back("POST", "/db/media/inline", "db", "post", true);
        m_routes.emplace_back("GET", "/exports/download", "exports", "get", true);
        m_routes.emplace_back("POST", "/exports", "exports", "post", true);
        m_routes.emplace_back("POST", "/slack/test", "slack", "post", true);
        m_routes.emplace_back("GET", "/tinybird/token", "tinybird", "get", true);
        m_routes.emplace_back("GET", "/featurebase/token", "featurebase", "get", true);
        m_routes.emplace_back("POST", "/session", "session", "post", true);
        m_routes.emplace_back("DELETE", "/session", "session", "delete", true);
        m_routes.emplace_back("POST", "/session/verify", "session", "post", true);
        m_routes.emplace_back("PUT", "/session/verify", "session", "put", true);
        m_routes.emplace_back("GET", "/identities", "identities", "get", true);
        m_routes.emplace_back("POST", "/authentication/password_reset", "authentication", "post", true);
        m_routes.emplace_back("PUT", "/authentication/password_reset", "authentication", "put", true);
        m_routes.emplace_back("POST", "/authentication/invitation", "authentication", "post", true);
        m_routes.emplace_back("GET", "/authentication/invitation", "authentication", "get", true);
        m_routes.emplace_back("POST", "/authentication/setup", "authentication", "post", true);
        m_routes.emplace_back("PUT", "/authentication/setup", "authentication", "put", true);
        m_routes.emplace_back("GET", "/authentication/setup", "authentication", "get", true);
        m_routes.emplace_back("POST", "/authentication/reset", "authentication", "post", true);
        m_routes.emplace_back("POST", "/images/upload", "images", "post", true);
        m_routes.emplace_back("POST", "/media/upload", "media", "post", true);
        m_routes.emplace_back("PUT", "/media/thumbnail/upload", "media", "put", true);
        m_routes.emplace_back("POST", "/files/upload", "files", "post", true);
        m_routes.emplace_back("GET", "/invites", "invites", "get", true);
        m_routes.emplace_back("GET", "/invites/:id", "invites", "get", true);
        m_routes.emplace_back("POST", "/invites", "invites", "post", true);
        m_routes.emplace_back("DELETE", "/invites/:id", "invites", "delete", true);
        m_routes.emplace_back("GET", "/redirects/download", "redirects", "get", true);
        m_routes.emplace_back("POST", "/redirects/upload", "redirects", "post", true);
        m_routes.emplace_back("GET", "/upgrades/", "upgrades", "get", true);
        m_routes.emplace_back("POST", "/upgrades/", "upgrades", "post", true);
        m_routes.emplace_back("GET", "/upgrades/:id/", "upgrades", "get", true);
        m_routes.emplace_back("POST", "/webhooks", "webhooks", "post", true);
        m_routes.emplace_back("PUT", "/webhooks/:id", "webhooks", "put", true);
        m_routes.emplace_back("DELETE", "/webhooks/:id", "webhooks", "delete", true);
        m_routes.emplace_back("GET", "/oembed", "oembed", "get", true);
        m_routes.emplace_back("GET", "/actions", "actions", "get", true);
        m_routes.emplace_back("GET", "/email_previews/posts/:id", "email_previews", "get", true);
        m_routes.emplace_back("POST", "/email_previews/posts/:id", "email_previews", "post", true);
        m_routes.emplace_back("GET", "/emails", "emails", "get", true);
        m_routes.emplace_back("GET", "/emails/:id", "emails", "get", true);
        m_routes.emplace_back("GET", "/emails/:id/status", "emails", "get", true);
        m_routes.emplace_back("PUT", "/emails/:id/retry", "emails", "put", true);
        m_routes.emplace_back("GET", "/emails/:id/batches", "emails", "get", true);
        m_routes.emplace_back("GET", "/emails/:id/recipient-failures", "emails", "get", true);
        m_routes.emplace_back("GET", "/emails/:id/analytics", "emails", "get", true);
        m_routes.emplace_back("PUT", "/emails/:id/analytics", "emails", "put", true);
        m_routes.emplace_back("DELETE", "/emails/analytics", "emails", "delete", true);
        m_routes.emplace_back("GET", "/snippets", "snippets", "get", true);
        m_routes.emplace_back("GET", "/snippets/:id", "snippets", "get", true);
        m_routes.emplace_back("POST", "/snippets", "snippets", "post", true);
        m_routes.emplace_back("PUT", "/snippets/:id", "snippets", "put", true);
        m_routes.emplace_back("DELETE", "/snippets/:id", "snippets", "delete", true);
        m_routes.emplace_back("GET", "/custom_theme_settings", "custom_theme_settings", "get", true);
        m_routes.emplace_back("PUT", "/custom_theme_settings", "custom_theme_settings", "put", true);
        m_routes.emplace_back("GET", "/newsletters", "newsletters", "get", true);
        m_routes.emplace_back("GET", "/newsletters/:id", "newsletters", "get", true);
        m_routes.emplace_back("POST", "/newsletters", "newsletters", "post", true);
        m_routes.emplace_back("PUT", "/newsletters/verifications/", "newsletters", "put", true);
        m_routes.emplace_back("PUT", "/newsletters/:id", "newsletters", "put", true);
        m_routes.emplace_back("GET", "/links", "links", "get", true);
        m_routes.emplace_back("PUT", "/links/bulk", "links", "put", true);
        m_routes.emplace_back("GET", "/recommendations", "recommendations", "get", true);
        m_routes.emplace_back("GET", "/recommendations/:id", "recommendations", "get", true);
        m_routes.emplace_back("POST", "/recommendations", "recommendations", "post", true);
        m_routes.emplace_back("POST", "/recommendations/check", "recommendations", "post", true);
        m_routes.emplace_back("PUT", "/recommendations/:id", "recommendations", "put", true);
        m_routes.emplace_back("DELETE", "/recommendations/:id", "recommendations", "delete", true);
        m_routes.emplace_back("GET", "/incoming_recommendations", "incoming_recommendations", "get", true);
        m_routes.emplace_back("GET", "/feedback/:id", "feedback", "get", true);
        m_routes.emplace_back("GET", "/search-index/posts", "search-index", "get", true);
        m_routes.emplace_back("GET", "/search-index/pages", "search-index", "get", true);
        m_routes.emplace_back("GET", "/search-index/tags", "search-index", "get", true);
        m_routes.emplace_back("GET", "/search-index/users", "search-index", "get", true);
    }

    [[nodiscard]] const std::vector<ApiRoute>& routes() const noexcept
    {
        return m_routes;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_routes.size();
    }

    [[nodiscard]] std::size_t content_count() const noexcept
    {
        return m_content_count;
    }

private:
    std::vector<ApiRoute> m_routes{};
    std::size_t m_content_count{20};
};

} // namespace blog::api
#ifdef CONGELADO_TEST
namespace blog::api::tests {
using namespace boost::ut;
suite<"blog_api_route_table"> blog_api_route_table_suite = []
{
    "contains all Ghost routes"_test = []
    {
        RouteTable table;
        expect(table.size() == 261);
        expect(table.content_count() == 20);
    };
};
} // namespace blog::api::tests
#endif
