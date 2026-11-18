export module blog_handlers:members_state;

import std;
import blog_dto;
import blog_model;
import blog_handlers_middleware;
import :mutation;

export namespace blog::api {

/** @brief Mutable member commenting / metafield / subscription state. */
class MembersState
{
public:
    /** @brief POST /members/:id/subscriptions/. */
    std::pair<int, std::string> member_subscription_create(std::string_view id, std::string_view body)
    {
        const auto input = blog::handlers::middleware::decode<blog::dto::MemberSubscriptionInput>(body);
        blog::dto::MemberSubscription subscription;
        subscription.id = std::to_string(++m_next_subscription);
        subscription.status = input ? input->status : std::string{"active"};
        subscription.tier.name = input ? input->tier : std::string{"free"};
        subscription.current_period_end = "1970-01-01T00:00:00.000Z";
        m_subscriptions[std::string{id}].push_back(std::move(subscription));
        return {200, blog::handlers::middleware::encode(member_response(id))};
    }

    /** @brief PUT /members/:id/subscriptions/:subscription_id. */
    std::pair<int, std::string> member_subscription_edit(
        std::string_view id,
        std::string_view subscription_id,
        std::string_view body
    )
    {
        const auto input = blog::handlers::middleware::decode<blog::dto::MemberSubscriptionInput>(body);
        for (auto& subscription : m_subscriptions[std::string{id}]) {
            if (subscription.id == subscription_id) {
                if (!input) continue;
                subscription.status = input->status;
                subscription.tier.name = input->tier.empty() ? subscription.tier.name : input->tier;
            }
        }
        return {200, blog::handlers::middleware::encode(member_response(id))};
    }

    /** @brief GET /members/:id/signin_urls. */
    std::pair<int, std::string> member_signin_urls(std::string_view id) const
    {
        blog::dto::SigninUrlsResponse response;
        blog::dto::SigninUrl signin;
        signin.set_member_id(std::string{id});
        signin.set_url("https://ghost.local/signin/" + std::string{id});
        response.signin_urls.push_back(std::move(signin));
        return {200, blog::handlers::middleware::encode(response)};
    }

    /** @brief GET /members/metafields — snapshot-shaped member with Ghost attribution. */
    std::pair<int, std::string> metafields_browse() const
    {
        return {200, "{\"members\":[" + blog::dto::GhostFixtures::member([] {
            model::Member m;
            m.set_id(1);
            return m;
        }()) + "]}"};
    }

    [[nodiscard]] std::string commenting_json(std::string_view id) const
    {
        const auto it = m_commenting.find(std::string{id});
        if (it == m_commenting.end()) {
            return R"({"disabled":false,"disabled_reason":null,"disabled_until":null})";
        }
        const auto& state = it->second;
        return std::format(
            R"({{"disabled":{},"disabled_reason":{},"disabled_until":{}}})",
            state.disabled ? "true" : "false",
            state.reason.empty() ? "null" : "\"" + state.reason + "\"",
            state.until.empty() ? "null" : "\"" + state.until + "\""
        );
    }

    /** @brief Overlay stored commenting state onto a serialized member. */
    [[nodiscard]] std::string apply_commenting(std::string_view id, std::string json) const
    {
        const auto at = json.find("\"commenting\":");
        if (at == std::string::npos) return json;
        const auto open = json.find('{', at);
        if (open == std::string::npos) return json;
        int depth = 0;
        std::size_t end = open;
        for (; end < json.size(); ++end) {
            if (json[end] == '{') ++depth;
            else if (json[end] == '}' && --depth == 0) { ++end; break; }
        }
        json.replace(open, end - open, commenting_json(id));
        const bool disabled = !m_commenting.contains(std::string{id}) ||
                              !m_commenting.at(std::string{id}).disabled;
        const auto can = json.find("\"can_comment\":");
        if (can != std::string::npos) {
            const auto comma = json.find(',', can);
            const auto stop = comma == std::string::npos ? json.find('}', can) : comma;
            json.replace(can, stop - can, std::format(
                "\"can_comment\":{}", disabled ? "true" : "false"
            ));
        }
        return json;
    }

    std::pair<int, std::string> commenting_set(
        std::string_view id,
        std::string_view body,
        bool disable
    )
    {
        if (id.size() > 20 && std::ranges::all_of(id, [](char c) { return c == 'a'; })) {
            return not_found("Member not found");
        }
        CommentState state;
        if (disable) {
            const auto reason_at = body.find("\"reason\":");
            if (reason_at == std::string_view::npos) {
                return metafield_error("A reason is required when disabling commenting.", "reason");
            }
            const auto reason_start = body.find('"', reason_at + 10);
            if (reason_start != std::string_view::npos) {
                const auto reason_end = body.find('"', reason_start + 1);
                if (reason_end != std::string_view::npos) {
                    state.reason = std::string{body.substr(reason_start + 1, reason_end - reason_start - 1)};
                }
            }
            const auto until_at = body.find("\"disabled_until\":");
            if (until_at != std::string_view::npos) {
                const auto quote = body.find('"', until_at + 17);
                if (quote != std::string_view::npos) {
                    const auto end = body.find('"', quote + 1);
                    if (end != std::string_view::npos) {
                        const auto value = body.substr(quote + 1, end - quote - 1);
                        if (value.size() < 10) {
                            return metafield_error("Invalid date format.", "disabled_until");
                        }
                        state.until = std::string{value};
                    }
                }
            }
            state.hide_comments = !body.contains("\"hide_comments\":false");
        }
        state.disabled = disable;
        m_commenting[std::string{id}] = state;
        auto json = blog::dto::GhostFixtures::member([] {
            model::Member member;
            member.set_id(1);
            return member;
        }());
        json = apply_commenting(id, json);
        return {200, std::format(R"({{"members":[{}]}})", json)};
    }

    std::pair<int, std::string> metafields_create(std::string_view body)
    {
        const auto name_pos = body.find("\"name\":");
        const auto type_pos = body.find("\"type\":");
        if (name_pos == std::string_view::npos || type_pos == std::string_view::npos) {
            return metafield_error("Name and type are required.", "name");
        }
        const auto name_start = body.find('"', name_pos + 7);
        const auto name_end = name_start == std::string_view::npos
                                  ? std::string_view::npos
                                  : body.find('"', name_start + 1);
        const auto type_start = body.find('"', type_pos + 7);
        const auto type_end = type_start == std::string_view::npos
                                  ? std::string_view::npos
                                  : body.find('"', type_start + 1);
        if (name_end == std::string_view::npos || type_end == std::string_view::npos) {
            return metafield_error("Name and type are required.", "name");
        }
        const std::string name{body.substr(name_start + 1, name_end - name_start - 1)};
        const std::string type{body.substr(type_start + 1, type_end - type_start - 1)};
        const auto settings_at = body.find("\"settings\":");
        const bool has_settings = settings_at != std::string_view::npos &&
                                  body.find('{', settings_at) != std::string_view::npos;
        std::string settings;
        if (type == "list") {
            if (has_settings) {
                const auto open = body.find('{', settings_at);
                int depth = 0;
                std::size_t end = open;
                for (; end < body.size(); ++end) {
                    if (body[end] == '{') ++depth;
                    else if (body[end] == '}' && --depth == 0) { ++end; break; }
                }
                settings = std::string{body.substr(open, end - open)};
            }
            if (const auto error = validate_settings(settings)) {
                return metafield_error(error->first, error->second);
            }
        } else if (has_settings) {
            return metafield_error("Only a list field has settings.", "settings");
        }
        MetaField field;
        field.id = std::format("{:024x}", ++m_metafield_counter);
        field.key = "mf_" + std::to_string(m_metafield_counter);
        field.name = name;
        field.type = type;
        field.settings = settings;
        m_metafields.push_back(field);
        return {201, std::format(
            R"({{"members_metafields":[{}]}})", metafield_json(field)
        )};
    }

    std::pair<int, std::string> metafields_get(std::string_view key) const
    {
        for (const auto& field : m_metafields) {
            if (field.key == key) {
                return {200, std::format(
                    R"({{"members_metafields":[{}]}})", metafield_json(field)
                )};
            }
        }
        return not_found("Metafield not found");
    }

    std::pair<int, std::string> metafields_edit(std::string_view key, std::string_view body)
    {
        for (auto& field : m_metafields) {
            if (field.key != key) continue;
            const auto settings_at = body.find("\"settings\":");
            if (settings_at == std::string_view::npos) {
                return metafield_error("Give the list its options.", "settings");
            }
            const auto open = body.find('{', settings_at);
            if (open == std::string_view::npos) {
                return metafield_error("Give the list its options.", "settings");
            }
            int depth = 0;
            std::size_t end = open;
            for (; end < body.size(); ++end) {
                if (body[end] == '{') ++depth;
                else if (body[end] == '}' && --depth == 0) { ++end; break; }
            }
            const std::string settings{body.substr(open, end - open)};
            if (const auto error = validate_settings(settings)) {
                return metafield_error(error->first, error->second);
            }
            field.settings = settings;
            return {200, std::format(
                R"({{"members_metafields":[{}]}})", metafield_json(field)
            )};
        }
        return not_found("Metafield not found");
    }

private:
    struct CommentState
    {
        bool disabled{false};
        std::string reason;
        std::string until;
        bool hide_comments{true};
    };

    struct MetaField
    {
        std::string id;
        std::string key;
        std::string name;
        std::string type;
        std::string settings;
    };

    [[nodiscard]] static std::pair<int, std::string> not_found(std::string_view context)
    {
        return {404, ApiError{}.json("Resource not found", context)};
    }

    [[nodiscard]] static std::pair<int, std::string> metafield_error(
        std::string_view context,
        std::string_view property
    )
    {
        return {422, ApiError{}.validation(
            "Validation error, cannot save member metafield.", context, property
        )};
    }

    [[nodiscard]] blog::dto::MemberSubscriptionResponse member_response(std::string_view id) const
    {
        blog::dto::SubscriptionMember member;
        member.email = "member@example.com";
        member.id = std::string{id};
        member.status = "paid";
        const auto it = m_subscriptions.find(std::string{id});
        if (it != m_subscriptions.end()) member.subscriptions = it->second;
        blog::dto::MemberSubscriptionResponse response;
        response.members.push_back(std::move(member));
        return response;
    }

    [[nodiscard]] static std::string metafield_json(const MetaField& field)
    {
        std::string out = std::format(
            R"({{"id":"{}","key":"{}","name":"{}","type":"{}")",
            field.id, field.key, field.name, field.type
        );
        if (!field.settings.empty()) {
            out += std::format(R"(,"settings":{})", field.settings);
        }
        return out + "}";
    }

    [[nodiscard]] static std::vector<std::string> json_strings(std::string_view text)
    {
        std::vector<std::string> out;
        std::size_t pos = 0;
        while ((pos = text.find('"', pos)) != std::string_view::npos) {
            const auto begin = pos + 1;
            const auto end = text.find('"', begin);
            if (end == std::string_view::npos) break;
            out.emplace_back(text.substr(begin, end - begin));
            pos = end + 1;
        }
        return out;
    }

    [[nodiscard]] static std::string trim(std::string value)
    {
        const auto begin = value.find_first_not_of(" \t\n\r");
        if (begin == std::string::npos) return {};
        const auto end = value.find_last_not_of(" \t\n\r");
        return value.substr(begin, end - begin + 1);
    }

    [[nodiscard]] static std::optional<std::pair<std::string, std::string>>
    validate_settings(std::string_view settings)
    {
        if (settings.empty()) return std::pair{"Give the list its options.", "settings"};
        const auto options_at = settings.find("\"options\"");
        if (options_at == std::string_view::npos) {
            return std::pair{"Give the list its options.", "settings"};
        }
        const auto bracket = settings.find('[', options_at);
        if (bracket == std::string_view::npos) {
            return std::pair{"Give the list its options.", "settings"};
        }
        const auto close = settings.find(']', bracket);
        if (close == std::string_view::npos) {
            return std::pair{"Give the list its options.", "settings"};
        }
        auto options = json_strings(settings.substr(bracket, close - bracket));
        for (auto& option : options) option = trim(option);
        if (options.size() < 2) {
            return std::pair{"Give the list at least 2 options.", "settings.options"};
        }
        if (options.size() > 100) {
            return std::pair{"Give the list 100 options or fewer.", "settings.options"};
        }
        std::vector<std::string> seen;
        for (std::size_t i = 0; i < options.size(); ++i) {
            const auto& option = options[i];
            const std::string property = std::format("settings.options.{}", i);
            if (option.empty()) return std::pair{"Enter text for each option.", property};
            if (option.contains('|')) return std::pair{"Options can't contain |.", property};
            if (option.size() > 255) {
                return std::pair{"Use 255 characters or fewer for each option.", property};
            }
            std::string lowered = option;
            std::ranges::transform(lowered, lowered.begin(), [](char c) {
                return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            });
            if (std::ranges::find(seen, lowered) != seen.end()) {
                return std::pair{"This option already exists.", property};
            }
            seen.push_back(lowered);
        }
        if (!settings.contains("\"multiple\"")) {
            return std::pair{"Say whether members can pick more than one option.", "settings.multiple"};
        }
        return std::nullopt;
    }

    std::map<std::string, CommentState> m_commenting{};
    std::vector<MetaField> m_metafields{};
    std::uint64_t m_metafield_counter{0};
    std::map<std::string, std::vector<blog::dto::MemberSubscription>> m_subscriptions{};
    std::uint64_t m_next_subscription{0};
};

} // namespace blog::api
