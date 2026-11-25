module;
#include <rfl/Generic.hpp>
export module blog_handlers:email_previews_dto_subresources;

import std;
import core_serde;
import core_query;

export namespace blog::dto::email_previews {

class EmailPreview
{
public:
    void set_subject(std::string value)
    {
        subject = std::move(value);
    }

    [[nodiscard]] const std::string& get_subject() const noexcept
    {
        return subject;
    }

    void set_html(std::string value)
    {
        html = std::move(value);
    }

    [[nodiscard]] const std::string& get_html() const noexcept
    {
        return html;
    }

    void set_plaintext(std::string value)
    {
        plaintext = std::move(value);
    }

    [[nodiscard]] const std::string& get_plaintext() const noexcept
    {
        return plaintext;
    }

    std::string subject{"Preview"};
    std::string html{"<p>Lorem ipsum.</p>"};
    std::string plaintext{"Lorem ipsum."};
};

class EmailPreviewsResponse
{
public:
    void set_email_previews(std::vector<EmailPreview> value)
    {
        email_previews = std::move(value);
    }

    [[nodiscard]] const std::vector<EmailPreview>& get_email_previews() const noexcept
    {
        return email_previews;
    }

    std::vector<EmailPreview> email_previews{};
};

class PasswordResponse
{
public:
    void set_password(rfl::Generic value)
    {
        password = std::move(value);
    }

    [[nodiscard]] const rfl::Generic& get_password() const noexcept
    {
        return password;
    }

    rfl::Generic password{};
};

class CommentSubResponse
{
public:
    void set_comments(std::vector<rfl::Generic> value)
    {
        comments = std::move(value);
    }

    [[nodiscard]] const std::vector<rfl::Generic>& get_comments() const noexcept
    {
        return comments;
    }

    std::vector<rfl::Generic> comments{};
};

/**
 * @brief Shared input/rendering helpers for both `/email_previews/posts/:id` routes.
 *
 * Kept in this already-registered partition (BUILD.bazel:24) so neither workflow
 * step needs a new partition or a BUILD edit. Both routes go through the same
 * `EmailController._getFrameData` service checks in Ghost, so that validation,
 * post/newsletter resolution and preview derivation live here once.
 */
namespace preview_support {

    using Object = serde::Value::Object;
    using Array = serde::Value::Array;

    struct Input
    {
        std::set<std::string> fields; // empty = all three preview fields
        std::optional<std::string> member_status;
        std::optional<std::string> member_tier;
        std::optional<std::string> member_segment;
        std::optional<std::string> newsletter;
    };

    struct InputError
    {
        std::string message;
        std::string property;
    };

    [[nodiscard]] inline std::string encode(std::string_view value)
    {
        return core_serde::JsonCodec{}.encode(std::string{value});
    }

    /** @brief Decoded query/body value for one key; `false` when absent. */
    [[nodiscard]] inline bool option(
        const std::map<std::string, std::string>& options,
        std::string_view key,
        std::string& out
    )
    {
        if (const auto it = options.find(std::string{key}); it != options.end()) {
            out = it->second;
            return true;
        }
        return false;
    }

    [[nodiscard]] inline const serde::Value* value_for(const Object* object, std::string_view key)
    {
        if (object == nullptr) {
            return nullptr;
        }
        for (const auto& [name, value]: *object) {
            if (name == key) {
                return &value;
            }
        }
        return nullptr;
    }

    [[nodiscard]] inline bool
    option_object(const Object& object, std::string_view key, std::string& out)
    {
        const auto* value = value_for(&object, key);
        if (value == nullptr) {
            return false;
        }
        if (const auto* text = std::get_if<std::string>(&value->get())) {
            out = *text;
            return true;
        }
        return false;
    }

    /** @brief Ghost rejects repeated member_* params: they parse as arrays, not strings. */
    [[nodiscard]] inline bool repeated_option(std::string_view path, std::string_view key)
    {
        const auto qpos = path.find('?');
        if (qpos == std::string_view::npos) {
            return false;
        }
        int seen = 0;
        for (auto part: std::views::split(path.substr(qpos + 1), '&')) {
            const std::string text{std::ranges::begin(part), std::ranges::end(part)};
            const auto eq = text.find('=');
            if (eq == std::string::npos) {
                continue;
            }
            if (std::string_view{text}.substr(0, eq) == key && ++seen > 1) {
                return true;
            }
        }
        return false;
    }

    /** @brief `core_query::parse` handles URL-decoding, allowlisting and `fields` projection. */
    inline void parse_options(std::string_view path, Input& out)
    {
        static constexpr std::string_view
            kAllowed[]{"fields", "member_status", "member_tier", "memberSegment", "newsletter"};
        const auto q = core::query::parse(path, kAllowed);
        out.fields = q.fields;

        std::string value;
        if (option(q.options, "member_status", value)) {
            out.member_status = value;
        }
        if (option(q.options, "member_tier", value)) {
            out.member_tier = value;
        }
        if (option(q.options, "memberSegment", value)) {
            out.member_segment = value;
        }
        if (option(q.options, "newsletter", value)) {
            out.newsletter = value;
        }
    }

    /** @brief POST reads the same service options from `data` as well as query options. */
    inline void parse_data_options(const Object* data, Input& out)
    {
        if (data == nullptr) {
            return;
        }
        std::string value;
        if (!out.member_status && option_object(*data, "member_status", value)) {
            out.member_status = value;
        }
        if (!out.member_tier && option_object(*data, "member_tier", value)) {
            out.member_tier = value;
        }
        if (!out.member_segment && option_object(*data, "memberSegment", value)) {
            out.member_segment = value;
        }
        if (!out.newsletter && option_object(*data, "newsletter", value)) {
            out.newsletter = value;
        }
    }

    /** @brief `EmailController._getFrameData` service-layer validation (422 envelopes). */
    [[nodiscard]] inline std::optional<InputError> validate(const Input& in, std::string_view path)
    {
        for (const auto& field: in.fields) {
            if (field != "html" && field != "plaintext" && field != "subject") {
                return InputError{"Invalid fields value", "fields"};
            }
        }
        if (repeated_option(path, "member_status") ||
            (in.member_status && *in.member_status != "free" && *in.member_status != "paid")) {
            return InputError{"member_status must be 'free' or 'paid'.", "member_status"};
        }
        if (repeated_option(path, "member_tier") || (in.member_tier && in.member_tier->empty())) {
            return InputError{"member_tier must be a single tier slug.", "member_tier"};
        }
        if (repeated_option(path, "memberSegment") ||
            (in.member_segment && *in.member_segment != "status:free" &&
             *in.member_segment != "status:-free")) {
            return InputError{
                "memberSegment is deprecated and only accepts 'status:free' or 'status:-free' — "
                "use member_status instead.",
                "memberSegment"
            };
        }
        return std::nullopt;
    }

    /** @brief Legacy `memberSegment` folds into member_status when member_status is absent. */
    inline void fold_legacy_segment(Input& in)
    {
        if (!in.member_status && in.member_segment) {
            in.member_status = (*in.member_segment == "status:free") ? "free" : "paid";
        }
    }

    [[nodiscard]] inline std::string field_string(const Object* object, std::string_view key)
    {
        if (object == nullptr) {
            return {};
        }
        std::string out;
        (void)option_object(*object, key, out);
        return out;
    }

    [[nodiscard]] inline const Object* as_object(const serde::Value* value)
    {
        return value != nullptr ? std::get_if<Object>(&value->get()) : nullptr;
    }

    [[nodiscard]] inline std::vector<serde::Value> decode_array(std::string_view text)
    {
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(std::string{text});
        const auto* array = parsed ? std::get_if<Array>(&parsed->get()) : nullptr;
        return array != nullptr ? std::vector<serde::Value>{array->begin(), array->end()}
                                : std::vector<serde::Value>{};
    }

    /** @brief `Post.findOne({id|uuid}, {status:'all'})` — no first-row fallback. */
    [[nodiscard]] inline std::optional<Object>
    select_post(std::string_view rows, std::string_view id)
    {
        for (const auto& row: decode_array(rows)) {
            const auto* post = as_object(&row);
            if (post != nullptr &&
                (field_string(post, "id") == id || field_string(post, "uuid") == id)) {
                return *post;
            }
        }
        return std::nullopt;
    }

    /** @brief Slug lookup with `require:true`, or the fixture default newsletter. */
    [[nodiscard]] inline std::optional<Object>
    select_newsletter(std::string_view rows, std::string_view slug)
    {
        auto newsletters = decode_array(rows);
        if (!slug.empty()) {
            for (const auto& row: newsletters) {
                const auto* newsletter = as_object(&row);
                if (newsletter != nullptr && field_string(newsletter, "slug") == slug) {
                    return *newsletter;
                }
            }
            return std::nullopt;
        }
        if (!newsletters.empty()) {
            if (const auto* first = as_object(&newsletters.front())) {
                return *first;
            }
        }
        return std::nullopt;
    }

    /** @brief Collect Lexical text nodes when the stored post has no rendered body. */
    inline void collect_text(const serde::Value& node, std::vector<std::string>& out)
    {
        if (const auto* object = as_object(&node)) {
            if (field_string(object, "type") == "text") {
                const auto text = field_string(object, "text");
                if (!text.empty()) {
                    out.push_back(text);
                }
            }
            for (const auto& [key, value]: *object) {
                if (key == "children" || key == "root") {
                    collect_text(value, out);
                }
            }
        } else if (const auto* array = std::get_if<Array>(&node.get())) {
            for (const auto& item: *array) {
                collect_text(item, out);
            }
        }
    }

    [[nodiscard]] inline std::pair<std::string, std::string> render_body(const Object* post)
    {
        if (post == nullptr) {
            return {};
        }
        std::vector<std::string> parts;
        const auto lexical = field_string(post, "lexical");
        if (!lexical.empty()) {
            if (const auto doc = core_serde::JsonCodec{}.decode<serde::Value>(lexical)) {
                collect_text(*doc, parts);
            }
        }
        std::string plaintext;
        for (std::size_t i = 0; i < parts.size(); ++i) {
            plaintext += (i == 0 ? std::string{} : std::string{"\n"}) + parts[i];
        }
        std::string html;
        for (const auto& part: parts) {
            html += "<p>" + part + "</p>";
        }
        return {html, plaintext};
    }

    /** @brief `EmailRenderer#getSubject` — posts_meta.email_subject, else post.title. */
    [[nodiscard]] inline std::string subject_of(const Object* post, const Object* newsletter)
    {
        if (post != nullptr) {
            const auto* meta = as_object(value_for(post, "posts_meta"));
            const auto email_subject = field_string(meta, "email_subject");
            if (!email_subject.empty()) {
                return email_subject;
            }
            const auto title = field_string(post, "title");
            if (!title.empty()) {
                return title;
            }
        }
        return field_string(newsletter, "name");
    }

    /** @brief Derive the Ghost `{subject,html,plaintext}` preview from stored JSON.
     *
     * The full Handlebars/Lexical renderer has no registered C++ primitive; the
     * fixture-backed derivation is the accepted residual. No fabricated
     * `Preview`/Lorem-ipsum values are emitted.
     */
    [[nodiscard]] inline std::string render(const Object* post, const Object* newsletter)
    {
        const auto rendered = render_body(post);
        auto html = field_string(post, "html");
        auto plaintext = field_string(post, "plaintext");
        if (plaintext.empty()) {
            plaintext = field_string(post, "excerpt");
        }
        if (plaintext.empty()) {
            plaintext = field_string(post, "custom_excerpt");
        }
        if (html.empty()) {
            html = rendered.first;
        }
        if (plaintext.empty()) {
            plaintext = rendered.second;
        }
        return "{\"subject\":" + encode(subject_of(post, newsletter)) +
               ",\"html\":" + encode(html) + ",\"plaintext\":" + encode(plaintext) + "}";
    }

    /** @brief Ghost `fields` option projects the preview object's keys. */
    [[nodiscard]] inline std::string
    project(std::string_view preview, const std::set<std::string>& fields)
    {
        if (fields.empty()) {
            return std::string{preview};
        }
        core::query::ParsedQuery q;
        q.fields = fields;
        const auto projected = core::query::apply("[" + std::string{preview} + "]", q, 15);
        auto rows = decode_array(projected);
        return rows.empty() ? std::string{"{}"} : core_serde::JsonCodec{}.encode(rows.front());
    }

    /** @brief Replace Ghost member tokens with the example member's values. */
    [[nodiscard]] inline std::string replace_definitions(
        std::string text,
        std::string_view first_name,
        std::string_view full_name,
        std::string_view email
    )
    {
        static constexpr std::string_view kTokens[]{
            "%%{first_name}%%",
            "%%{name}%%",
            "%%{email}%%",
            "{first_name}",
            "{name}",
            "{email}"
        };
        const std::string_view values[]{first_name, full_name, email, first_name, full_name, email};
        for (std::size_t i = 0; i < std::size(kTokens); ++i) {
            for (;;) {
                const auto pos = text.find(kTokens[i]);
                if (pos == std::string::npos) {
                    break;
                }
                text.replace(pos, kTokens[i].size(), values[i]);
            }
        }
        return text;
    }

} // namespace preview_support
} // namespace blog::dto::email_previews

template<>
struct serde::Serializable<blog::dto::email_previews::EmailPreview>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "subject",
                &blog::dto::email_previews::EmailPreview::get_subject,
                &blog::dto::email_previews::EmailPreview::set_subject,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "html",
                &blog::dto::email_previews::EmailPreview::get_html,
                &blog::dto::email_previews::EmailPreview::set_html,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "plaintext",
                &blog::dto::email_previews::EmailPreview::get_plaintext,
                &blog::dto::email_previews::EmailPreview::set_plaintext,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "email_preview";
    }
};

template<>
struct serde::Serializable<blog::dto::email_previews::EmailPreviewsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "email_previews",
            &blog::dto::email_previews::EmailPreviewsResponse::get_email_previews,
            &blog::dto::email_previews::EmailPreviewsResponse::set_email_previews,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "email_previews_response";
    }
};

template<>
struct serde::Serializable<blog::dto::email_previews::PasswordResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "password",
            &blog::dto::email_previews::PasswordResponse::get_password,
            &blog::dto::email_previews::PasswordResponse::set_password,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "password_response";
    }
};

template<>
struct serde::Serializable<blog::dto::email_previews::CommentSubResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "comments",
            &blog::dto::email_previews::CommentSubResponse::get_comments,
            &blog::dto::email_previews::CommentSubResponse::set_comments,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "comment_sub_response";
    }
};
