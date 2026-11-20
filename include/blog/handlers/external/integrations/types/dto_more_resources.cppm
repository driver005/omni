module;
#include <rfl/Generic.hpp>
export module blog_handlers:integrations_dto_more_resources;

import std;
import core_serde;
import interfaces;
import blog_handlers_middleware;

export namespace blog::dto::integrations {

class RecordDto
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_slug(std::string value) { slug = std::move(value); }
    [[nodiscard]] const std::string& get_slug() const noexcept { return slug; }
    std::string id{"1"};
    std::string name{"Ghost Record"};
    std::string slug{"ghost-record"};
};

class AutomatedEmailsResponse
{
public:
    void set_automated_emails(std::vector<RecordDto> value) { automated_emails = std::move(value); }
    [[nodiscard]] const std::vector<RecordDto>& get_automated_emails() const noexcept { return automated_emails; }
    std::vector<RecordDto> automated_emails{};
};

class CustomThemeSettingsResponse
{
public:
    void set_custom_theme_settings(std::vector<RecordDto> value) { custom_theme_settings = std::move(value); }
    [[nodiscard]] const std::vector<RecordDto>& get_custom_theme_settings() const noexcept { return custom_theme_settings; }
    std::vector<RecordDto> custom_theme_settings{};
};

class IntegrationsResponse
{
public:
    void set_integrations(std::vector<RecordDto> value) { integrations = std::move(value); }
    [[nodiscard]] const std::vector<RecordDto>& get_integrations() const noexcept { return integrations; }
    std::vector<RecordDto> integrations{};
};

class OembedResponse
{
public:
    void set_html(std::string value) { html = std::move(value); }
    [[nodiscard]] const std::string& get_html() const noexcept { return html; }
    void set_url(std::string value) { url = std::move(value); }
    [[nodiscard]] const std::string& get_url() const noexcept { return url; }
    std::string html{"<iframe src=\"https://ghost.local/embed\"></iframe>"};
    std::string url{"https://ghost.local/embed"};
};

class FeedbackResponse
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_message(std::string value) { message = std::move(value); }
    [[nodiscard]] const std::string& get_message() const noexcept { return message; }
    std::string id{"1"};
    std::string message{"Ghost feedback"};
};

class SearchPagesResponse
{
public:
    void set_pages(std::vector<RecordDto> value) { pages = std::move(value); }
    [[nodiscard]] const std::vector<RecordDto>& get_pages() const noexcept { return pages; }
    std::vector<RecordDto> pages{};
};

class SearchPostsResponse
{
public:
    void set_posts(std::vector<RecordDto> value) { posts = std::move(value); }
    [[nodiscard]] const std::vector<RecordDto>& get_posts() const noexcept { return posts; }
    std::vector<RecordDto> posts{};
};

class SearchTagsResponse
{
public:
    void set_tags(std::vector<RecordDto> value) { tags = std::move(value); }
    [[nodiscard]] const std::vector<RecordDto>& get_tags() const noexcept { return tags; }
    std::vector<RecordDto> tags{};
};

class SearchAuthorsResponse
{
public:
    void set_authors(std::vector<RecordDto> value) { authors = std::move(value); }
    [[nodiscard]] const std::vector<RecordDto>& get_authors() const noexcept { return authors; }
    std::vector<RecordDto> authors{};
};

class SearchUsersResponse
{
public:
    void set_users(std::vector<RecordDto> value) { users = std::move(value); }
    [[nodiscard]] const std::vector<RecordDto>& get_users() const noexcept { return users; }
    std::vector<RecordDto> users{};
};

} // namespace blog::dto::integrations


template<> struct serde::Serializable<blog::dto::integrations::RecordDto>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::dto::integrations::RecordDto::get_id, &blog::dto::integrations::RecordDto::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &blog::dto::integrations::RecordDto::get_name, &blog::dto::integrations::RecordDto::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"slug", &blog::dto::integrations::RecordDto::get_slug, &blog::dto::integrations::RecordDto::set_slug, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "record_dto"; }
};

template<> struct serde::Serializable<blog::dto::integrations::AutomatedEmailsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"automated_emails", &blog::dto::integrations::AutomatedEmailsResponse::get_automated_emails,
                                           &blog::dto::integrations::AutomatedEmailsResponse::set_automated_emails,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "automated_emails_response"; }
};
template<> struct serde::Serializable<blog::dto::integrations::CustomThemeSettingsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"custom_theme_settings", &blog::dto::integrations::CustomThemeSettingsResponse::get_custom_theme_settings,
                                           &blog::dto::integrations::CustomThemeSettingsResponse::set_custom_theme_settings,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "custom_theme_settings_response"; }
};
template<> struct serde::Serializable<blog::dto::integrations::IntegrationsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"integrations", &blog::dto::integrations::IntegrationsResponse::get_integrations,
                                           &blog::dto::integrations::IntegrationsResponse::set_integrations,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "integrations_response"; }
};

template<> struct serde::Serializable<blog::dto::integrations::SearchPagesResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"pages", &blog::dto::integrations::SearchPagesResponse::get_pages,
                                           &blog::dto::integrations::SearchPagesResponse::set_pages,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "search_pages"; }
};
template<> struct serde::Serializable<blog::dto::integrations::SearchPostsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"posts", &blog::dto::integrations::SearchPostsResponse::get_posts,
                                           &blog::dto::integrations::SearchPostsResponse::set_posts,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "search_posts"; }
};
template<> struct serde::Serializable<blog::dto::integrations::SearchTagsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"tags", &blog::dto::integrations::SearchTagsResponse::get_tags,
                                           &blog::dto::integrations::SearchTagsResponse::set_tags,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "search_tags"; }
};
template<> struct serde::Serializable<blog::dto::integrations::SearchUsersResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"users", &blog::dto::integrations::SearchUsersResponse::get_users,
                                           &blog::dto::integrations::SearchUsersResponse::set_users,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "search_users"; }
};

template<> struct serde::Serializable<blog::dto::integrations::SearchAuthorsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<"authors", &blog::dto::integrations::SearchAuthorsResponse::get_authors,
                                           &blog::dto::integrations::SearchAuthorsResponse::set_authors,
                                           serde::FieldOptions{}>{}};
    }
    static constexpr std::string_view table_name() { return "search_authors"; }
};

template<> struct serde::Serializable<blog::dto::integrations::OembedResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"html", &blog::dto::integrations::OembedResponse::get_html, &blog::dto::integrations::OembedResponse::set_html, serde::FieldOptions{}>{},
            serde::FieldDesc<"url", &blog::dto::integrations::OembedResponse::get_url, &blog::dto::integrations::OembedResponse::set_url, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "oembed_response"; }
};

template<> struct serde::Serializable<blog::dto::integrations::FeedbackResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::dto::integrations::FeedbackResponse::get_id, &blog::dto::integrations::FeedbackResponse::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"message", &blog::dto::integrations::FeedbackResponse::get_message, &blog::dto::integrations::FeedbackResponse::set_message, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "feedback_response"; }
};

// ---------------------------------------------------------------------------
// Strict-parity support for the integrations endpoints.
//
// These helpers live in the folder's already-registered DTO partition so no new
// module partition (and no shared BUILD change) is required. Store-dependent
// helpers are templates over the store type and are instantiated in the step
// modules, which import :domain_all.
//
// Ghost references:
//   api/endpoints/integrations.js, services/integrations/integrations-service.js,
//   models/integration.js, models/api-key.js,
//   utils/serializers/input/integrations.js,
//   utils/serializers/output/mappers/integrations.ts.
// ---------------------------------------------------------------------------
export namespace blog::handlers::integrations::support {

using Value = serde::Value;
using ValueObject = Value::Object;
using ValueArray = Value::Array;

// rfl::Object keeps find() private; count()+at() are the public lookup pair.
[[nodiscard]] inline Value* lookup(ValueObject& object, std::string_view key)
{
    const std::string name{key};
    return object.count(name) == 0 ? nullptr : &object.at(name);
}
[[nodiscard]] inline const Value* lookup(const ValueObject& object, std::string_view key)
{
    const std::string name{key};
    return object.count(name) == 0 ? nullptr : &object.at(name);
}

[[nodiscard]] inline std::optional<Value> decode(std::string_view text)
{
    return core_serde::JsonCodec{}.decode<Value>(text);
}
[[nodiscard]] inline std::string encode(const Value& value)
{
    return core_serde::JsonCodec{}.encode(value);
}
[[nodiscard]] inline std::string quote(std::string_view value)
{
    return core_serde::JsonCodec{}.encode(std::string{value});
}
[[nodiscard]] inline std::string unquote(std::string_view raw)
{
    if (raw.size() >= 2 && raw.front() == '"' && raw.back() == '"') {
        const auto value = decode(raw);
        if (value) {
            if (const auto* text = std::get_if<std::string>(&value->get())) return *text;
        }
    }
    return std::string{raw};
}
[[nodiscard]] inline std::string scalar(std::string_view raw)
{
    const auto value = decode(raw);
    if (!value) return {};
    if (const auto* text = std::get_if<std::string>(&value->get())) return *text;
    if (const auto* number = std::get_if<std::int64_t>(&value->get())) return std::to_string(*number);
    return {};
}
[[nodiscard]] inline std::map<std::string, std::string> object(std::string_view text)
{
    std::map<std::string, std::string> out;
    const auto value = decode(text);
    if (!value) return out;
    const auto* obj = std::get_if<ValueObject>(&value->get());
    if (obj == nullptr) return out;
    for (const auto& [key, field] : *obj) out[key] = encode(field);
    return out;
}
[[nodiscard]] inline std::vector<std::string> array(std::string_view text)
{
    std::vector<std::string> out;
    const auto value = decode(text);
    if (!value) return out;
    const auto* items = std::get_if<ValueArray>(&value->get());
    if (items == nullptr) return out;
    for (const auto& item : *items) out.push_back(encode(item));
    return out;
}
[[nodiscard]] inline std::string field(std::string_view object_text, std::string_view key)
{
    const auto fields = object(object_text);
    const auto it = fields.find(std::string{key});
    return it == fields.end() ? std::string{} : it->second;
}
[[nodiscard]] inline std::string with_field(std::string_view object_text,
                                            std::string_view key,
                                            std::string_view raw_value)
{
    auto value = decode(object_text);
    if (!value) return std::string{object_text};
    auto* obj = std::get_if<ValueObject>(&value->get());
    if (obj == nullptr) return std::string{object_text};
    const auto replacement = decode(raw_value);
    (*obj)[std::string{key}] = replacement ? *replacement : Value{};
    return encode(*value);
}
[[nodiscard]] inline std::string make_object(const std::map<std::string, std::string>& fields)
{
    std::string out = "{";
    bool first = true;
    for (const auto& [key, raw] : fields) {
        if (!first) out += ',';
        out += quote(key) + ':' + (raw.empty() ? std::string{"null"} : raw);
        first = false;
    }
    return out + "}";
}
[[nodiscard]] inline std::string make_array(const std::vector<std::string>& items)
{
    std::string out = "[";
    for (std::size_t i = 0; i < items.size(); ++i) out += (i ? "," : "") + items[i];
    return out + "]";
}

// Ghost collection envelope (blog_handlers:mapping::wrap). Kept local because the
// mapping partitions are imported by other groups but are not registered in the
// blog_handlers BUILD partition list; no new-partition edit is allowed here.
[[nodiscard]] inline std::string wrap(std::string_view resource, std::string_view rows,
                                      std::string_view meta = {})
{
    std::string body{rows};
    if (body.empty() || body.front() != '[') {
        body = (body.size() >= 2 && body.front() == '{') ? "[" + body + "]" : std::string{"[]"};
    }
    std::string out = "{\"" + std::string{resource} + "\":" + std::move(body);
    if (!meta.empty()) out += ",\"meta\":" + std::string{meta};
    return out + '}';
}

// Ghost-shaped error envelopes (errors[0].message/type/property).
[[nodiscard]] inline std::string ghost_error(std::string_view message,
                                             std::string_view type,
                                             std::string_view property = {})
{
    static std::uint64_t counter = 0;
    const std::string id = std::format("{:032x}", ++counter);
    return std::format(
        R"({{"errors":[{{"id":"{}","code":null,"context":null,"details":null,"ghostErrorCode":null,"help":null,"message":{},"property":{},"type":{}}}]}})",
        id, quote(message), property.empty() ? std::string{"null"} : quote(property), quote(type));
}
[[nodiscard]] inline std::string not_found_integration()
{
    return ghost_error("Integration not found.", "NotFoundError");
}
[[nodiscard]] inline std::string not_found_api_key()
{
    return ghost_error("ApiKey not found.", "NotFoundError");
}
[[nodiscard]] inline std::string internal_error()
{
    return ghost_error("Internal Server Error", "InternalServerError");
}
[[nodiscard]] inline std::string api_token_blocked()
{
    return ghost_error("API tokens do not have permission to access this endpoint",
                       "NoPermissionError");
}

// Ghost generateSlug: lowercase, non-alphanumerics become '-', collapse/trim.
[[nodiscard]] inline std::string slugify(std::string_view name)
{
    std::string out;
    bool pending_dash = false;
    for (const unsigned char c : name) {
        if (std::isalnum(c)) {
            if (pending_dash && !out.empty()) out += '-';
            out += static_cast<char>(std::tolower(c));
            pending_dash = false;
        } else {
            pending_dash = true;
        }
    }
    while (!out.empty() && out.back() == '-') out.pop_back();
    return out;
}

// security.secret.create(type): content = 13 bytes (26 hex), admin/default = 32 bytes (64 hex).
[[nodiscard]] inline std::string new_secret(std::string_view type)
{
    static std::random_device rd;
    static constexpr char kHex[] = "0123456789abcdef";
    const int bytes = type == "content" ? 13 : 32;
    std::string out;
    out.reserve(static_cast<std::size_t>(bytes) * 2);
    for (int i = 0; i < bytes; ++i) {
        const unsigned byte = rd() & 0xFF;
        out += kHex[(byte >> 4) & 0xF];
        out += kHex[byte & 0xF];
    }
    return out;
}

[[nodiscard]] inline std::string now_iso()
{
    using namespace std::chrono;
    const auto tick = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    const long long seconds = tick / 1000;
    const long long millis = tick % 1000;
    long long z = seconds / 86400 + 719468;
    const long long era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    long long year = static_cast<long long>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    const unsigned day = doy - (153 * mp + 2) / 5 + 1;
    const unsigned month = mp < 10 ? mp + 3 : mp - 9;
    year += (month <= 2) ? 1 : 0;
    const unsigned sod = static_cast<unsigned>(seconds % 86400);
    return std::format("{:04}-{:02}-{:02}T{:02}:{:02}:{:02}.{:03}Z",
                       year, month, day, sod / 3600, (sod % 3600) / 60, sod % 60, millis);
}

// output/mappers/integrations.ts: prefix every admin api_key secret with "<id>:".
[[nodiscard]] inline std::string prefix_admin_secrets(std::string_view json)
{
    auto value = decode(json);
    if (!value) return std::string{json};
    ValueArray* rows = nullptr;
    if (auto* items = std::get_if<ValueArray>(&value->get())) rows = items;
    if (auto* obj = std::get_if<ValueObject>(&value->get())) {
        rows = nullptr;
        // Single object: promote into a one-element array for uniform handling.
        ValueArray single;
        single.push_back(*obj);
        value = Value{std::move(single)};
        rows = std::get_if<ValueArray>(&value->get());
    }
    if (rows == nullptr) return std::string{json};
    for (auto& row : *rows) {
        auto* integration = std::get_if<ValueObject>(&row.get());
        if (integration == nullptr) continue;
        Value* keys_value = lookup(*integration, "api_keys");
        if (keys_value == nullptr) continue;
        auto* keys = std::get_if<ValueArray>(&keys_value->get());
        if (keys == nullptr) continue;
        for (auto& key : *keys) {
            auto* api_key = std::get_if<ValueObject>(&key.get());
            if (api_key == nullptr) continue;
            const Value* type_value = lookup(*api_key, "type");
            if (type_value == nullptr) continue;
            const auto* type = std::get_if<std::string>(&type_value->get());
            if (type == nullptr || *type != "admin") continue;
            const Value* id_value = lookup(*api_key, "id");
            Value* secret_value = lookup(*api_key, "secret");
            if (id_value == nullptr || secret_value == nullptr) continue;
            const auto* id = std::get_if<std::string>(&id_value->get());
            const auto* secret = std::get_if<std::string>(&secret_value->get());
            if (id == nullptr || secret == nullptr) continue;
            *secret_value = Value{*id + ":" + *secret};
        }
    }
    const std::string encoded = encode(*value);
    // Restore the single-object shape.
    if (json.empty() || json.front() != '[') {
        const auto items = array(encoded);
        return items.empty() ? std::string{"{}"} : items.front();
    }
    return encoded;
}

[[nodiscard]] inline bool valid_include(std::string_view csv)
{
    for (auto part : csv | std::views::split(',')) {
        const std::string token{std::ranges::begin(part), std::ranges::end(part)};
        if (token != "api_keys" && token != "webhooks") return false;
    }
    return true;
}
[[nodiscard]] inline bool include_has(std::string_view csv, std::string_view token)
{
    for (auto part : csv | std::views::split(',')) {
        const std::string value{std::ranges::begin(part), std::ranges::end(part)};
        if (value == token) return true;
    }
    return false;
}

[[nodiscard]] inline std::string tail_id(std::string_view path)
{
    auto bare = path.substr(0, path.find('?'));
    while (bare.size() > 1 && bare.back() == '/') bare.remove_suffix(1);
    const auto slash = bare.find_last_of('/');
    return std::string{slash == std::string_view::npos ? bare : bare.substr(slash + 1)};
}

// The refresh path carries two ids: /integrations/<id>/api_key/<keyid>/refresh.
[[nodiscard]] inline std::pair<std::string, std::string> path_ids(std::string_view path)
{
    auto bare = path.substr(0, path.find('?'));
    while (bare.size() > 1 && bare.back() == '/') bare.remove_suffix(1);
    const auto marker = bare.find("/integrations/");
    if (marker == std::string_view::npos) return {};
    const auto rest = bare.substr(marker + 14);
    const auto slash = rest.find('/');
    const std::string id{slash == std::string_view::npos ? rest : rest.substr(0, slash)};
    std::string keyid;
    const auto key_marker = rest.find("/api_key/");
    if (key_marker != std::string_view::npos) {
        keyid = std::string{rest.substr(key_marker + 9)};
        const auto key_slash = keyid.find('/');
        if (key_slash != std::string::npos) keyid.resize(key_slash);
    }
    return {id, keyid};
}

[[nodiscard]] inline std::map<std::string, std::string> query_options(
    std::string_view path, std::initializer_list<std::string_view> keys)
{
    std::map<std::string, std::string> out;
    const auto question = path.find('?');
    if (question == std::string_view::npos) return out;
    for (auto pair : path.substr(question + 1) | std::views::split('&')) {
        const std::string text{std::ranges::begin(pair), std::ranges::end(pair)};
        const auto eq = text.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = text.substr(0, eq);
        if (std::ranges::find(keys, std::string_view{key}) != std::ranges::end(keys)) {
            out[key] = text.substr(eq + 1);
        }
    }
    return out;
}

// ---- stateful helpers (instantiated where :domain_all is imported) --------

template<class Store>
[[nodiscard]] std::string api_keys_for(Store& store, std::string_view integration_id)
{
    std::vector<std::string> out;
    for (const auto& row : array(store.resources().list("api_keys"))) {
        if (unquote(field(row, "integration_id")) == integration_id) out.push_back(row);
    }
    return make_array(out);
}

template<class Store>
[[nodiscard]] std::string webhooks_for(Store& store, std::string_view integration_id)
{
    std::vector<std::string> out;
    for (const auto& row : array(store.resources().list("webhooks"))) {
        if (unquote(field(row, "integration_id")) == integration_id) out.push_back(row);
    }
    return make_array(out);
}

template<class Store>
[[nodiscard]] std::string create_api_keys(Store& store, std::string_view integration_id)
{
    std::vector<std::string> created;
    for (auto type : {"content", "admin"}) {
        const std::map<std::string, std::string> row{
            {"type", quote(type)},
            {"secret", quote(new_secret(type))},
            {"integration_id", quote(integration_id)},
            {"created_at", quote(now_iso())},
            {"updated_at", quote(now_iso())}};
        const std::string id = store.resources().create("api_keys", make_object(row));
        const auto stored = store.resources().get("api_keys", id);
        created.push_back(stored.empty() ? with_field(make_object(row), "id", quote(id)) : stored);
    }
    return make_array(created);
}

template<class Store>
[[nodiscard]] std::string create_webhooks(Store& store, std::string_view integration_id,
                                          std::string_view webhooks_json)
{
    std::vector<std::string> created;
    for (const auto& hook : array(webhooks_json)) {
        auto row = object(hook);
        row["integration_id"] = quote(integration_id);
        row.erase("id");
        const std::string id = store.resources().create("webhooks", make_object(row));
        const auto stored = store.resources().get("webhooks", id);
        created.push_back(stored.empty() ? with_field(make_object(row), "id", quote(id)) : stored);
    }
    return make_array(created);
}

// Ghost relation editing on save: drop the old webhook rows, insert the new set.
template<class Store>
[[nodiscard]] std::string replace_webhooks(Store& store, std::string_view integration_id,
                                           std::string_view webhooks_json)
{
    for (const auto& hook : array(store.resources().list("webhooks"))) {
        const auto id = unquote(field(hook, "id"));
        if (!id.empty() && unquote(field(hook, "integration_id")) == integration_id) {
            store.resources().destroy("webhooks", id);
        }
    }
    return create_webhooks(store, integration_id, webhooks_json);
}

template<class Store>
[[nodiscard]] std::string find_api_key(Store& store, std::string_view keyid)
{
    for (const auto& row : array(store.resources().list("api_keys"))) {
        if (unquote(field(row, "id")) == keyid) return row;
    }
    return {};
}

// ApiKeyModel.refreshSecret: generate a type-appropriate secret and persist it.
template<class Store>
[[nodiscard]] std::string rotate_api_key(Store& store, std::string_view keyid)
{
    for (const auto& row : array(store.resources().list("api_keys"))) {
        if (unquote(field(row, "id")) != keyid) continue;
        std::string updated = with_field(row, "secret",
                                         quote(new_secret(unquote(field(row, "type")))));
        updated = with_field(updated, "updated_at", quote(now_iso()));
        store.resources().edit("api_keys", unquote(field(updated, "id")), updated);
        return updated;
    }
    return {};
}

// Attach default relations (api_keys always; webhooks on request) to an integration row.
template<class Store>
[[nodiscard]] std::string embed_relations(Store& store, std::string_view integration_json,
                                          std::string_view include, bool force_webhooks = false)
{
    const std::string id = unquote(field(integration_json, "id"));
    std::string out = with_field(integration_json, "api_keys", api_keys_for(store, id));
    if (force_webhooks || include_has(include, "webhooks")) {
        out = with_field(out, "webhooks", webhooks_for(store, id));
    }
    return out;
}

// ---- admin API gate -------------------------------------------------------

/**
 * @brief Stand-in for Ghost's authAdminApi identity step. A valid bearer token
 *        authenticates an Administrator in the ADMIN_API context; explicit
 *        x-blog-* headers always win so a deliberately weaker test role is
 *        still denied by the ghost authorizer.
 */
inline void prepare_principal(interfaces::io::IRequest& request)
{
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::kGhostRoleHeader;
    if (request.get_authorization().empty()) return;
    if (request.find_header(kGhostRoleHeader).empty()) {
        request.set_header(kGhostRoleHeader, "Administrator");
    }
    if (request.find_header(kGhostContextHeader).empty()) {
        request.set_header(kGhostContextHeader, "ADMIN_API");
    }
}

/**
 * @brief mw.authAdminApi + permissions:true + tokenPermissionCheck.
 *  Integrations are absent from the integration-token allowlist
 *  (admin/middleware.js:55-99), so INTERNAL/SYSTEM (integration) tokens are
 *  blocked with 403 apiTokenBlocked. All others run require_ghost_permission
 *  against docName 'integrations'.
 *  @return true when authorized; false when a Ghost error envelope was sent.
 */
[[nodiscard]] inline bool authorized(interfaces::io::IRequest& request,
                                     interfaces::io::IResponse& response,
                                     std::function<void()> send,
                                     std::string_view method)
{
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::GhostPermissionRegistry;
    prepare_principal(request);

    const std::string_view context = request.find_header(kGhostContextHeader);
    if (context == "INTERNAL" || context == "SYSTEM") {
        std::vector<std::byte> bytes;
        const std::string body = api_token_blocked();
        for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
        response.set_status(interfaces::io::types::Status::FORBIDDEN);
        response.set_body(std::move(bytes));
        send();
        return false;
    }

    bool denied = false;
    auto sentinel = [&denied, send = std::move(send)]() mutable {
        denied = true;
        send();
    };
    GhostPermissionRegistry::set_active(&request, &response, sentinel);
    blog::handlers::middleware::require_ghost_permission("integrations", method);
    GhostPermissionRegistry::clear_active();
    return !denied;
}

} // namespace blog::handlers::integrations::support
