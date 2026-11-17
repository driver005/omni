module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <expected>

export module blog_api:endpoints:members_admin;

import std;
import blog_api:types;
import blog_database:database;
import core_router;
import interfaces;

export namespace ghost::blog::endpoints {

using json = nlohmann::json;

/**
 * Members Admin Controller - Matches Ghost's members.js controller
 */
class MembersAdminController {
public:
    explicit MembersAdminController(std::shared_ptr<IDatabase> database) noexcept
        : m_database{std::move(database)} {}

    struct BrowseOptions {
        std::string m_include{};
        std::string m_filter{};
        std::string m_fields{};
        int m_limit = 15;
        std::string m_order{};
        int m_page = 1;
        bool m_debug = false;
        
        BrowseOptions() noexcept = default;
        BrowseOptions& add_include(std::string&& include) noexcept { m_include = std::move(include); return *this; }
        BrowseOptions& add_filter(std::string&& filter) noexcept { m_filter = std::move(filter); return *this; }
        BrowseOptions& add_fields(std::string&& fields) noexcept { m_fields = std::move(fields); return *this; }
        BrowseOptions& add_limit(int limit) noexcept { m_limit = limit; return *this; }
        BrowseOptions& add_order(std::string&& order) noexcept { m_order = std::move(order); return *this; }
        BrowseOptions& add_page(int page) noexcept { m_page = page; return *this; }
        BrowseOptions& add_debug(bool debug) noexcept { m_debug = debug; return *this; }
        
        void set_include(std::string&& include) noexcept { m_include = std::move(include); }
        void set_filter(std::string&& filter) noexcept { m_filter = std::move(filter); }
        void set_fields(std::string&& fields) noexcept { m_fields = std::move(fields); }
        void set_limit(int limit) noexcept { m_limit = limit; }
        void set_order(std::string&& order) noexcept { m_order = std::move(order); }
        void set_page(int page) noexcept { m_page = page; }
        void set_debug(bool debug) noexcept { m_debug = debug; }
        
        std::string& get_include() noexcept { return m_include; }
        std::string& get_filter() noexcept { return m_filter; }
        std::string& get_fields() noexcept { return m_fields; }
        int& get_limit() noexcept { return m_limit; }
        std::string& get_order() noexcept { return m_order; }
        int& get_page() noexcept { return m_page; }
        bool& get_debug() noexcept { return m_debug; }
        
        const std::string& get_include() const noexcept { return m_include; }
        const std::string& get_filter() const noexcept { return m_filter; }
        const std::string& get_fields() const noexcept { return m_fields; }
        int get_limit() const noexcept { return m_limit; }
        const std::string& get_order() const noexcept { return m_order; }
        int get_page() const noexcept { return m_page; }
        bool get_debug() const noexcept { return m_debug; }
    };

    // Member struct for admin API
    struct Member {
        std::string m_id{};
        std::string m_uuid{};
        std::string m_email{};
        std::string m_name{};
        std::string m_note{};
        std::string m_avatar_image{};
        std::string m_status{};
        std::string m_visibility{};
        std::string m_comp_email{};
        std::string m_comp_source{};
        std::string m_subscribed{};
        std::string m_created_at{};
        std::string m_updated_at{};
        std::string m_last_login{};
        std::string m_last_seen{};
        std::string m_last_signin{};
        int m_logins_count = 0;
        json m_meta{};
        std::vector<std::string> m_labels{};
        std::vector<json> m_subscriptions{};
        
        Member() noexcept = default;
        
        Member& add_email(std::string&& email) noexcept { m_email = std::move(email); return *this; }
        Member& add_name(std::string&& name) noexcept { m_name = std::move(name); return *this; }
        Member& add_status(std::string&& status) noexcept { m_status = std::move(status); return *this; }
        
        void set_id(std::string&& id) noexcept { m_id = std::move(id); }
        void set_uuid(std::string&& uuid) noexcept { m_uuid = std::move(uuid); }
        void set_email(std::string&& email) noexcept { m_email = std::move(email); }
        void set_name(std::string&& name) noexcept { m_name = std::move(name); }
        void set_note(std::string&& note) noexcept { m_note = std::move(note); }
        void set_avatar_image(std::string&& img) noexcept { m_avatar_image = std::move(img); }
        void set_status(std::string&& status) noexcept { m_status = std::move(status); }
        void set_visibility(std::string&& vis) noexcept { m_visibility = std::move(vis); }
        void set_comp_email(std::string&& email) noexcept { m_comp_email = std::move(email); }
        void set_comp_source(std::string&& source) noexcept { m_comp_source = std::move(source); }
        void set_subscribed(std::string&& sub) noexcept { m_subscribed = std::move(sub); }
        void set_created_at(std::string&& created) noexcept { m_created_at = std::move(created); }
        void set_updated_at(std::string&& updated) noexcept { m_updated_at = std::move(updated); }
        void set_last_login(std::string&& login) noexcept { m_last_login = std::move(login); }
        void set_last_seen(std::string&& seen) noexcept { m_last_seen = std::move(seen); }
        void set_last_signin(std::string&& signin) noexcept { m_last_signin = std::move(signin); }
        void set_logins_count(int count) noexcept { m_logins_count = count; }
        void set_meta(json&& meta) noexcept { m_meta = std::move(meta); }
        void set_labels(std::vector<std::string>&& labels) noexcept { m_labels = std::move(labels); }
        void set_subscriptions(std::vector<json>&& subs) noexcept { m_subscriptions = std::move(subs); }
        
        std::string& get_id() noexcept { return m_id; }
        std::string& get_uuid() noexcept { return m_uuid; }
        std::string& get_email() noexcept { return m_email; }
        std::string& get_name() noexcept { return m_name; }
        std::string& get_note() noexcept { return m_note; }
        std::string& get_avatar_image() noexcept { return m_avatar_image; }
        std::string& get_status() noexcept { return m_status; }
        std::string& get_visibility() noexcept { return m_visibility; }
        std::string& get_comp_email() noexcept { return m_comp_email; }
        std::string& get_comp_source() noexcept { return m_comp_source; }
        std::string& get_subscribed() noexcept { return m_subscribed; }
        std::string& get_created_at() noexcept { return m_created_at; }
        std::string& get_updated_at() noexcept { return m_updated_at; }
        std::string& get_last_login() noexcept { return m_last_login; }
        std::string& get_last_seen() noexcept { return m_last_seen; }
        std::string& get_last_signin() noexcept { return m_last_signin; }
        int& get_logins_count() noexcept { return m_logins_count; }
        json& get_meta() noexcept { return m_meta; }
        std::vector<std::string>& get_labels() noexcept { return m_labels; }
        std::vector<json>& get_subscriptions() noexcept { return m_subscriptions; }
        
        const std::string& get_id() const noexcept { return m_id; }
        const std::string& get_uuid() const noexcept { return m_uuid; }
        const std::string& get_email() const noexcept { return m_email; }
        const std::string& get_name() const noexcept { return m_name; }
        const std::string& get_note() const noexcept { return m_note; }
        const std::string& get_avatar_image() const noexcept { return m_avatar_image; }
        const std::string& get_status() const noexcept { return m_status; }
        const std::string& get_visibility() const noexcept { return m_visibility; }
        const std::string& get_comp_email() const noexcept { return m_comp_email; }
        const std::string& get_comp_source() const noexcept { return m_comp_source; }
        const std::string& get_subscribed() const noexcept { return m_subscribed; }
        const std::string& get_created_at() const noexcept { return m_created_at; }
        const std::string& get_updated_at() const noexcept { return m_updated_at; }
        const std::string& get_last_login() const noexcept { return m_last_login; }
        const std::string& get_last_seen() const noexcept { return m_last_seen; }
        const std::string& get_last_signin() const noexcept { return m_last_signin; }
        int get_logins_count() const noexcept { return m_logins_count; }
        const json& get_meta() const noexcept { return m_meta; }
        const std::vector<std::string>& get_labels() const noexcept { return m_labels; }
        const std::vector<json>& get_subscriptions() const noexcept { return m_subscriptions; }
        
        json to_json() const noexcept {
            json j;
            j["id"] = m_id;
            j["uuid"] = m_uuid;
            j["email"] = m_email;
            j["name"] = m_name;
            j["note"] = m_note;
            j["avatar_image"] = m_avatar_image;
            j["status"] = m_status;
            j["visibility"] = m_visibility;
            j["comp_email"] = m_comp_email;
            j["comp_source"] = m_comp_source;
            j["subscribed"] = m_subscribed;
            j["created_at"] = m_created_at;
            j["updated_at"] = m_updated_at;
            j["last_login"] = m_last_login;
            j["last_seen"] = m_last_seen;
            j["last_signin"] = m_last_signin;
            j["logins_count"] = m_logins_count;
            j["meta"] = m_meta;
            j["labels"] = m_labels;
            j["subscriptions"] = m_subscriptions;
            return j;
        }
    };

    // Stats structs
    struct MemberStats {
        int m_total = 0;
        int m_paid = 0;
        int m_free = 0;
        int m_comp = 0;
        int m_none = 0;
        
        MemberStats() noexcept = default;
        MemberStats& add_total(int total) noexcept { m_total = total; return *this; }
        MemberStats& add_paid(int paid) noexcept { m_paid = paid; return *this; }
        MemberStats& add_free(int free) noexcept { m_free = free; return *this; }
        MemberStats& add_comp(int comp) noexcept { m_comp = comp; return *this; }
        MemberStats& add_none(int none) noexcept { m_none = none; return *this; }
        
        void set_total(int total) noexcept { m_total = total; }
        void set_paid(int paid) noexcept { m_paid = paid; }
        void set_free(int free) noexcept { m_free = free; }
        void set_comp(int comp) noexcept { m_comp = comp; }
        void set_none(int none) noexcept { m_none = none; }
        
        int& get_total() noexcept { return m_total; }
        int& get_paid() noexcept { return m_paid; }
        int& get_free() noexcept { return m_free; }
        int& get_comp() noexcept { return m_comp; }
        int& get_none() noexcept { return m_none; }
        
        int get_total() const noexcept { return m_total; }
        int get_paid() const noexcept { return m_paid; }
        int get_free() const noexcept { return m_free; }
        int get_comp() const noexcept { return m_comp; }
        int get_none() const noexcept { return m_none; }
        
        json to_json() const noexcept {
            json j;
            j["total"] = m_total;
            j["paid"] = m_paid;
            j["free"] = m_free;
            j["comp"] = m_comp;
            j["none"] = m_none;
            return j;
        }
    };

    struct MrrStats {
        std::string m_currency{"USD"};
        int m_mrr = 0;
        int m_arr = 0;
        
        MrrStats() noexcept = default;
        MrrStats& add_currency(std::string&& currency) noexcept { m_currency = std::move(currency); return *this; }
        MrrStats& add_mrr(int mrr) noexcept { m_mrr = mrr; return *this; }
        MrrStats& add_arr(int arr) noexcept { m_arr = arr; return *this; }
        
        void set_currency(std::string&& currency) noexcept { m_currency = std::move(currency); }
        void set_mrr(int mrr) noexcept { m_mrr = mrr; }
        void set_arr(int arr) noexcept { m_arr = arr; }
        
        std::string& get_currency() noexcept { return m_currency; }
        int& get_mrr() noexcept { return m_mrr; }
        int& get_arr() noexcept { return m_arr; }
        
        const std::string& get_currency() const noexcept { return m_currency; }
        int get_mrr() const noexcept { return m_mrr; }
        int get_arr() const noexcept { return m_arr; }
        
        json to_json() const noexcept {
            json j;
            j["currency"] = m_currency;
            j["mrr"] = m_mrr;
            j["arr"] = m_arr;
            return j;
        }
    };

    // Browse members
    std::expected<ApiResponse<std::vector<Member>>, std::string> browse(const BrowseOptions& options) noexcept {
        // Placeholder implementation
        std::vector<Member> members;
        return ApiResponse<std::vector<Member>>::ok(std::move(members));
    }

    // Read member
    std::expected<ApiResponse<Member>, std::string> read(const std::string& id) noexcept {
        Member member;
        member.set_id(id);
        return ApiResponse<Member>::ok(std::move(member));
    }

    // Add member
    std::expected<ApiResponse<Member>, std::string> add(const Member& member) noexcept {
        return ApiResponse<Member>::created(member);
    }

    // Edit member
    std::expected<ApiResponse<Member>, std::string> edit(const std::string& id, const Member& member) noexcept {
        Member updated = member;
        updated.set_id(id);
        return ApiResponse<Member>::ok(std::move(updated));
    }

    // Destroy member
    std::expected<ApiResponse<bool>, std::string> destroy(const std::string& id) noexcept {
        return ApiResponse<bool>::ok(true);
    }

    // Bulk operations
    std::expected<ApiResponse<std::vector<Member>>, std::string> bulk_destroy(const std::vector<std::string>& ids) noexcept {
        return ApiResponse<std::vector<Member>>::ok({});
    }

    std::expected<ApiResponse<std::vector<Member>>, std::string> bulk_edit(const json& bulk_data) noexcept {
        return ApiResponse<std::vector<Member>>::ok({});
    }

    // Stats
    std::expected<ApiResponse<MemberStats>, std::string> member_stats() noexcept {
        MemberStats stats;
        stats.set_total(0);
        stats.set_paid(0);
        stats.set_free(0);
        stats.set_comp(0);
        stats.set_none(0);
        return ApiResponse<MemberStats>::ok(std::move(stats));
    }

    std::expected<ApiResponse<MrrStats>, std::string> mrr_stats() noexcept {
        MrrStats stats;
        stats.set_currency("USD");
        stats.set_mrr(0);
        stats.set_arr(0);
        return ApiResponse<MrrStats>::ok(std::move(stats));
    }

    // Activity feed
    std::expected<ApiResponse<json>, std::string> activity_feed(const BrowseOptions& options) noexcept {
        return ApiResponse<json>::ok(json{});
    }

    // Export/Import CSV
    std::expected<ApiResponse<json>, std::string> export_csv(const BrowseOptions& options) noexcept {
        json data;
        data["filename"] = "members_export.csv";
        data["data"] = json::array();
        return ApiResponse<json>::ok(std::move(data));
    }

    std::expected<ApiResponse<json>, std::string> import_csv(const std::string& file_path, const json& mapping) noexcept {
        json result;
        result["import_id"] = "import_123";
        result["total"] = 0;
        return ApiResponse<json>::ok(std::move(result));
    }

    // Stripe Connect
    std::expected<ApiResponse<json>, std::string> stripe_connect_auth() noexcept {
        return ApiResponse<json>::ok(json{{"url", "https://connect.stripe.com/oauth/authorize"}});
    }

    // Member subscriptions
    std::expected<ApiResponse<Member>, std::string> create_subscription(const std::string& member_id, const json& subscription_data) noexcept {
        Member member;
        member.set_id(member_id);
        return ApiResponse<Member>::ok(std::move(member));
    }

    std::expected<ApiResponse<Member>, std::string> edit_subscription(const std::string& member_id, const std::string& subscription_id, const json& subscription_data) noexcept {
        Member member;
        member.set_id(member_id);
        return ApiResponse<Member>::ok(std::move(member));
    }

    // Member signin URLs
    std::expected<ApiResponse<json>, std::string> read_signin_urls(const std::string& member_id) noexcept {
        return ApiResponse<json>::ok(json{{"signin_url", "https://example.com/signin"}});
    }

    // Member commenting
    std::expected<ApiResponse<Member>, std::string> disable_commenting(const std::string& member_id) noexcept {
        Member member;
        member.set_id(member_id);
        return ApiResponse<Member>::ok(std::move(member));
    }

    std::expected<ApiResponse<Member>, std::string> enable_commenting(const std::string& member_id) noexcept {
        Member member;
        member.set_id(member_id);
        return ApiResponse<Member>::ok(std::move(member));
    }

    // Member sessions
    std::expected<ApiResponse<bool>, std::string> logout(const std::string& member_id) noexcept {
        return ApiResponse<bool>::ok(true);
    }

    // Email suppression
    std::expected<ApiResponse<bool>, std::string> delete_email_suppression(const std::string& member_id) noexcept {
        return ApiResponse<bool>::ok(true);
    }

private:
    std::shared_ptr<IDatabase> m_database;
};

/**
 * Register members admin routes
 */
void register_members_admin_routes(core::router::RouterContext<256, 64, 10>& router_ctx,
                                   std::shared_ptr<IDatabase> database) {
    MembersAdminController controller{std::move(database)};
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/members/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/members/:id/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/members/stats/count/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/members/stats/mrr/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/members/events/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/members/upload/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/members/stripe_connect/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/members/:id/subscriptions/"}
        .post([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/members/:id/subscriptions/:subscription_id/"}
        .put([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/members/:id/signin_urls/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/members/:id/commenting/disable/"}
        .post([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/members/:id/commenting/enable/"}
        .post([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/members/:id/sessions/"}
        .delete_([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/members/:id/suppression/"}
        .delete_([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
}

} // namespace ghost::blog::endpoints