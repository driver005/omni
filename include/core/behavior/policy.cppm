export module core_behavior:policy;

import std;
import interfaces;
import core_error;

export namespace core::behavior {

struct AuthSpec
{
    std::string_view permission{};
    bool allow_connected{false};
    bool allow_platform{false};
    bool allow_connected_scope_operation{false};
    bool allow_platform_self_operation{false};
};

struct Scope
{
    std::string merchant_id{};
    std::string profile_id{};
    std::string org_id{};
    std::string connected_merchant_id{};
};

struct CallContext
{
    std::string operation{};
    std::string merchant_id{};
    std::string profile_id{};
    std::string org_id{};
    std::string connected_merchant_id{};
    std::string client_secret{};
    std::vector<std::string> path_segments{};
    std::unordered_map<std::string, std::string> query{};
    std::unordered_map<std::string, std::string> headers{};
    std::unordered_map<std::string, std::string> claims{};
};

template<class P>
concept RequestPolicy = requires(
    P& policy,
    interfaces::io::IRequest& req,
    std::string_view op,
    const AuthSpec& spec,
    const Scope& scope
) {
    {
        policy.authorize(req, op, spec)
    } -> std::convertible_to<std::optional<core::error::AppError>>;
    { policy.scope(req) } -> std::convertible_to<Scope>;
    { policy.context(req) } -> std::convertible_to<CallContext>;
    { policy.scope_error(scope) } -> std::convertible_to<std::optional<core::error::AppError>>;
};

template<RequestPolicy Policy>
class RequestSupport
{
public:
    explicit RequestSupport(Policy& policy) noexcept :
        m_policy{policy}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req, std::string_view operation, const AuthSpec& spec) const
    {
        return m_policy.authorize(req, operation, spec);
    }

    [[nodiscard]] Scope scope(interfaces::io::IRequest& req) const
    {
        return m_policy.scope(req);
    }

    [[nodiscard]] CallContext context(interfaces::io::IRequest& req) const
    {
        return m_policy.context(req);
    }

    [[nodiscard]] std::optional<core::error::AppError> scope_error(const Scope& scope) const
    {
        return m_policy.scope_error(scope);
    }

    [[nodiscard]] std::string path(interfaces::io::IRequest& req, std::size_t index) const
    {
        const auto raw = req.get_path();
        const auto question = raw.find('?');
        const auto path = question == std::string_view::npos ? raw : raw.substr(0, question);
        std::size_t current{0};
        std::size_t start{0};
        while (start < path.size()) {
            while (start < path.size() && path[start] == '/') {
                ++start;
            }
            if (start >= path.size()) {
                break;
            }
            const auto end = path.find('/', start);
            const auto segment = end == std::string_view::npos ? path.substr(start)
                                                               : path.substr(start, end - start);
            if (current == index) {
                return std::string{segment};
            }
            ++current;
            if (end == std::string_view::npos) {
                break;
            }
            start = end + 1;
        }
        return {};
    }

    [[nodiscard]] std::optional<std::string>
    query(interfaces::io::IRequest& req, std::string_view key) const
    {
        const auto raw = req.get_path();
        const auto question = raw.find('?');
        if (question == std::string_view::npos) {
            return std::nullopt;
        }
        auto tail = raw.substr(question + 1);
        while (!tail.empty()) {
            const auto amp = tail.find('&');
            const auto pair = amp == std::string_view::npos ? tail : tail.substr(0, amp);
            const auto eq = pair.find('=');
            const auto name = eq == std::string_view::npos ? pair : pair.substr(0, eq);
            if (name == key) {
                auto value =
                    eq == std::string_view::npos ? std::string_view{} : pair.substr(eq + 1);
                return std::string{value};
            }
            if (amp == std::string_view::npos) {
                break;
            }
            tail = tail.substr(amp + 1);
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::string>
    header(interfaces::io::IRequest& req, std::string_view name) const
    {
        const auto value = req.find_header(name);
        if (value.empty()) {
            return std::nullopt;
        }
        return std::string{value};
    }

private:
    Policy& m_policy;
};

} // namespace core::behavior
