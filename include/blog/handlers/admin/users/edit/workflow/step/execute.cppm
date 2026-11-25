export module blog_handlers:users_edit_step;
import :domain_all;
import :users_dto_subresources;
import std;
import core_workflow;
import interfaces;
import core_auth;
import :outbox_shim;
import core_query;
import blog_handlers_middleware;

export namespace blog::handlers::users::edit::workflow {

namespace local_mapping {
    [[nodiscard]] inline std::string
    wrap(std::string_view resource, std::string_view rows, std::string_view meta = {})
    {
        return "{\"" + std::string{resource} + "\":" + std::string{rows} +
               (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
    }

    [[nodiscard]] inline std::string clean(std::string_view json, std::string_view)
    {
        return std::string{json};
    }

    [[nodiscard]] inline std::string pagination(
        std::size_t total,
        std::size_t limit,
        std::size_t page,
        std::size_t pages,
        std::string_view prev,
        std::string_view next
    )
    {
        return std::format(
            R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
            total,
            limit,
            page,
            pages,
            prev.empty() ? std::string{"null"} : std::string{prev},
            next.empty() ? std::string{"null"} : std::string{next}
        );
    }
} // namespace local_mapping

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"users_edit_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const auto id = last_segment(in.get_path());
        if (id.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}.validation("Validation error", "An id is required", "id")}}
            };
        }
        static constexpr std::string_view kOptions[]{"id", "include"};
        const auto q = core::query::parse(in.get_path(), kOptions);
        for (const auto& include: q.include) {
            if (!allowed_include(include)) {
                return core::workflow::StepResponse<Output>{
                    {{"status", "422"},
                     {"body",
                      blog::api::ApiError{}
                          .validation("Validation error", "Invalid include", "include")}}
                };
            }
        }

        std::string body = in.body_as_string().empty() ? "{}" : in.body_as_string();
        const auto top = blog::api::JsonBody{}.parse_raw(body);
        if (const auto it = top.find("users"); it != top.end()) {
            const auto entries = blog::api::parse_array(it->second);
            if (!entries.empty()) {
                body = blog::api::record_to_json(entries.front());
            }
        }

        // Ghost `permissions.unsafeAttrs:['status','roles']` and the users:edit check.
        const auto principal = make_principal(in);
        const auto authorizer = core::auth::ghost::GhostAuthorizer{};
        const auto decision = authorizer.authorize(principal, "users", "edit", id);
        if (!decision.allowed) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(decision.status)}, {"body", decision.error_json}}
            };
        }
        const auto changed = blog::api::JsonBody{}.parse_raw(body);
        for (const auto* attr: {"status", "roles"}) {
            if (!changed.contains(attr)) {
                continue;
            }
            const auto unsafe = authorizer.unsafe_attrs(principal, attr, changed.at(attr));
            if (!unsafe.allowed) {
                return core::workflow::StepResponse<Output>{
                    {{"status", std::to_string(unsafe.status)}, {"body", unsafe.error_json}}
                };
            }
        }

        std::string invalidate = "false";
        const auto before = ctx.store().find_by_resource("users", id);
        if (before) {
            const auto old = blog::api::JsonBody{}.parse_raw(*before);
            for (const auto attr: public_attrs) {
                const auto a = old.find(std::string{attr});
                const auto b = changed.find(std::string{attr});
                if ((a == old.end()) != (b == changed.end()) ||
                    (a != old.end() && a->second != b->second)) {
                    invalidate = "/*";
                    break;
                }
            }
        }

        const auto mutation = ctx.store().edit("users", id, body);
        if (!mutation.ok()) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(mutation.status())},
                 {"body", mutation.body()},
                 {"invalidate", invalidate}}
            };
        }
        if (invalidate == "/*") {
            blog::handlers::outbox::global_outbox().emit(
                blog::handlers::outbox::Event{
                    .type = "user.public_attrs_changed",
                    .payload = std::format(
                        R"({{"id":{},"cache_invalidate":"/*"}})",
                        blog::api::JsonBody{}.quote(id)
                    ),
                    .aggregate_id = id
                }
            );
        }
        const int status = mutation.status() == 201 ? 200 : mutation.status();
        const auto edited = ctx.store().find_by_resource("users", id);
        const auto model = edited ? local_mapping::clean(*edited, "user-wire") : std::string{"{}"};
        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(status)},
             {"body", local_mapping::wrap("users", model)},
             {"invalidate", invalidate}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static constexpr std::string_view public_attrs[]{
        "name",    "slug",     "profile_image", "cover_image", "bio",
        "website", "location", "facebook",      "twitter",     "mastodon",
        "youtube", "linkedin", "bluesky",       "instagram",   "tiktok",
        "threads", "status",   "visibility",    "meta_title",  "meta_description"
    };

    static bool allowed_include(std::string_view value)
    {
        return value == "count.posts" || value == "permissions" || value == "roles" ||
               value == "roles.permissions";
    }

    static core::auth::ghost::Principal make_principal(interfaces::io::IRequest& in)
    {
        auto principal = blog::handlers::middleware::principal_from_request(in);
        auto subject = std::string{in.find_header("x-blog-user-id")};
        if (subject.empty() && !in.get_authorization().empty()) {
            subject = "1";
        }
        principal.id = std::move(subject);
        return principal;
    }

    static std::string last_segment(std::string_view path)
    {
        path = path.substr(0, path.find('?'));
        while (path.size() > 1 && path.back() == '/') {
            path.remove_suffix(1);
        }
        const auto slash = path.find_last_of('/');
        return std::string{slash == std::string_view::npos ? path : path.substr(slash + 1)};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::users::edit::workflow
