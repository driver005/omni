module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <expected>

export module blog_api:endpoints:members_public;

import std;
import blog_api:types;
import blog_database:database;
import core_router;
import interfaces;

export namespace ghost::blog::endpoints {

using json = nlohmann::json;

/**
 * Members Public Controller - Matches Ghost's comments-members.js and members-account.js controllers
 */
class MembersPublicController {
public:
    explicit MembersPublicController(std::shared_ptr<IDatabase> database) noexcept
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

    // Member struct for public API
    struct Member {
        std::string m_id{};
        std::string m_uuid{};
        std::string m_email{};
        std::string m_name{};
        std::string m_avatar_image{};
        std::string m_status{};
        std::string m_visibility{};
        std::string m_created_at{};
        std::string m_updated_at{};
        json m_meta{};
        std::vector<std::string> m_labels{};
        std::vector<json> m_subscriptions{};
        
        Member() noexcept = default;
        
        json to_json() const noexcept {
            json j;
            j["id"] = m_id;
            j["uuid"] = m_uuid;
            j["email"] = m_email;
            j["name"] = m_name;
            j["avatar_image"] = m_avatar_image;
            j["status"] = m_status;
            j["visibility"] = m_visibility;
            j["created_at"] = m_created_at;
            j["updated_at"] = m_updated_at;
            j["meta"] = m_meta;
            j["labels"] = m_labels;
            j["subscriptions"] = m_subscriptions;
            return j;
        }
    };

    // Browse members (for newsletters)
    std::expected<ApiResponse<std::vector<Member>>, std::string> browse(const BrowseOptions& options) noexcept {
        std::vector<Member> members;
        return ApiResponse<std::vector<Member>>::ok(std::move(members));
    }

    // Read member account
    std::expected<ApiResponse<Member>, std::string> read_account(const std::string& member_id) noexcept {
        Member member;
        member.set_id(member_id);
        return ApiResponse<Member>::ok(std::move(member));
    }

    // Update member account
    std::expected<ApiResponse<Member>, std::string> edit_account(const std::string& member_id, const json& data) noexcept {
        Member member;
        member.set_id(member_id);
        return ApiResponse<Member>::ok(std::move(member));
    }

    // Delete member account
    std::expected<ApiResponse<bool>, std::string> destroy_account(const std::string& member_id) noexcept {
        return ApiResponse<bool>::ok(true);
    }

    // Member sessions
    std::expected<ApiResponse<bool>, std::string> logout(const std::string& member_id) noexcept {
        return ApiResponse<bool>::ok(true);
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

    // Email suppression
    std::expected<ApiResponse<bool>, std::string> delete_email_suppression(const std::string& member_id) noexcept {
        return ApiResponse<bool>::ok(true);
    }

private:
    std::shared_ptr<IDatabase> m_database;
};

/**
 * Register members public routes (Members API)
 */
void register_members_public_routes(core::router::RouterContext<256, 64, 10>& router_ctx,
                                    std::shared_ptr<IDatabase> database) {
    MembersPublicController controller{std::move(database)};
    
    router_ctx.add_route(core::router::Route<8, 10>{"/members/members/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/members/account/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .put([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        })
        .delete_([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        })
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/members/account/sessions/"}
        .delete_([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/members/account/signin_urls/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/members/account/commenting/disable/"}
        .post([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/members/account/commenting/enable/"}
        .post([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/members/account/suppression/"}
        .delete_([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
}

} // namespace ghost::blog::endpoints