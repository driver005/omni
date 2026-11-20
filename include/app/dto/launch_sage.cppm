module;
export module app_dto:launch_sage;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::launch_sage {

class LaunchSageResponse
{
public:
    void set_handoff_url(std::string value) { handoff_url = std::move(value); }
    [[nodiscard]] const std::string& get_handoff_url() const noexcept { return handoff_url; }
    void set_expires_at(std::optional<std::string> value) { expires_at = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_expires_at() const noexcept { return expires_at; }
    std::string handoff_url{};
    std::optional<std::string> expires_at{};
};

class ScopeEntry
{
public:
    void set_entity_type(std::string value) { entity_type = std::move(value); }
    [[nodiscard]] const std::string& get_entity_type() const noexcept { return entity_type; }
    void set_entity_id(std::string value) { entity_id = std::move(value); }
    [[nodiscard]] const std::string& get_entity_id() const noexcept { return entity_id; }
    void set_path(std::unordered_map<std::string, std::string> value) { path = std::move(value); }
    [[nodiscard]] const std::unordered_map<std::string, std::string>& get_path() const noexcept { return path; }
    void set_constraints(std::unordered_map<std::string, serde::Value> value) { constraints = std::move(value); }
    [[nodiscard]] const std::unordered_map<std::string, serde::Value>& get_constraints() const noexcept { return constraints; }
    std::string entity_type{};
    std::string entity_id{};
    std::unordered_map<std::string, std::string> path{};
    std::unordered_map<std::string, serde::Value> constraints{};
};

class SageSessionRequest
{
public:
    void set_user_id(std::string value) { user_id = std::move(value); }
    [[nodiscard]] const std::string& get_user_id() const noexcept { return user_id; }
    void set_launch_merchant_id(std::string value) { launch_merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_launch_merchant_id() const noexcept { return launch_merchant_id; }
    void set_launch_profile_id(std::string value) { launch_profile_id = std::move(value); }
    [[nodiscard]] const std::string& get_launch_profile_id() const noexcept { return launch_profile_id; }
    void set_org_id(std::string value) { org_id = std::move(value); }
    [[nodiscard]] const std::string& get_org_id() const noexcept { return org_id; }
    void set_role(std::string value) { role = std::move(value); }
    [[nodiscard]] const std::string& get_role() const noexcept { return role; }
    void set_source(std::string value) { source = std::move(value); }
    [[nodiscard]] const std::string& get_source() const noexcept { return source; }
    void set_subject_jti(std::optional<std::string> value) { subject_jti = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_subject_jti() const noexcept { return subject_jti; }
    void set_scope(std::vector<ScopeEntry> value) { scope = std::move(value); }
    [[nodiscard]] const std::vector<ScopeEntry>& get_scope() const noexcept { return scope; }
    std::string user_id{};
    std::string launch_merchant_id{};
    std::string launch_profile_id{};
    std::string org_id{};
    std::string role{};
    std::string source{};
    std::optional<std::string> subject_jti{};
    std::vector<ScopeEntry> scope{};
};
} // namespace app::dto::launch_sage

template<>
struct serde::Serializable<app::dto::launch_sage::LaunchSageResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"handoff_url", &app::dto::launch_sage::LaunchSageResponse::get_handoff_url, &app::dto::launch_sage::LaunchSageResponse::set_handoff_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"expires_at", &app::dto::launch_sage::LaunchSageResponse::get_expires_at, &app::dto::launch_sage::LaunchSageResponse::set_expires_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "launch_sage_response"; }
};

template<>
struct serde::Serializable<app::dto::launch_sage::ScopeEntry>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"entity_type", &app::dto::launch_sage::ScopeEntry::get_entity_type, &app::dto::launch_sage::ScopeEntry::set_entity_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"entity_id", &app::dto::launch_sage::ScopeEntry::get_entity_id, &app::dto::launch_sage::ScopeEntry::set_entity_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"path", &app::dto::launch_sage::ScopeEntry::get_path, &app::dto::launch_sage::ScopeEntry::set_path, serde::FieldOptions{}>{},
            serde::FieldDesc<"constraints", &app::dto::launch_sage::ScopeEntry::get_constraints, &app::dto::launch_sage::ScopeEntry::set_constraints, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "scope_entry"; }
};

template<>
struct serde::Serializable<app::dto::launch_sage::SageSessionRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"user_id", &app::dto::launch_sage::SageSessionRequest::get_user_id, &app::dto::launch_sage::SageSessionRequest::set_user_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"launch_merchant_id", &app::dto::launch_sage::SageSessionRequest::get_launch_merchant_id, &app::dto::launch_sage::SageSessionRequest::set_launch_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"launch_profile_id", &app::dto::launch_sage::SageSessionRequest::get_launch_profile_id, &app::dto::launch_sage::SageSessionRequest::set_launch_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"org_id", &app::dto::launch_sage::SageSessionRequest::get_org_id, &app::dto::launch_sage::SageSessionRequest::set_org_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"role", &app::dto::launch_sage::SageSessionRequest::get_role, &app::dto::launch_sage::SageSessionRequest::set_role, serde::FieldOptions{}>{},
            serde::FieldDesc<"source", &app::dto::launch_sage::SageSessionRequest::get_source, &app::dto::launch_sage::SageSessionRequest::set_source, serde::FieldOptions{}>{},
            serde::FieldDesc<"subject_jti", &app::dto::launch_sage::SageSessionRequest::get_subject_jti, &app::dto::launch_sage::SageSessionRequest::set_subject_jti, serde::FieldOptions{}>{},
            serde::FieldDesc<"scope", &app::dto::launch_sage::SageSessionRequest::get_scope, &app::dto::launch_sage::SageSessionRequest::set_scope, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "sage_session_request"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::launch_sage::tests {
using namespace boost::ut;
suite<"app_dto_launch_sage"> app_dto_launch_sage_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::launch_sage::LaunchSageResponse>::fields())> > 0);
    };
};
}
#endif
