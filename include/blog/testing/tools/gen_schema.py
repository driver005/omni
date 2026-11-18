"""Regenerates ghost_schema.cppm from the Jest snapshot schemas.

Run snapshot_normalize.py first to write /tmp/opencode/ghost_schema.json, then
extract_more-style collection for the other resources into
/tmp/opencode/ghost_schema_more.json.
"""
import json, pathlib

schema = json.load(open('/tmp/opencode/ghost_schema.json'))
schema.update(json.load(open('/tmp/opencode/ghost_schema_more.json')))


def types(v):
    order = ['null', 'string', 'number', 'boolean', 'object', 'array']
    return [t for t in order if t in v] or ['any']


def spec_lines(resource):
    out = []
    for k, v in sorted(schema[resource]['fields'].items()):
        ts = types(v)
        joined = ','.join('"' + t + '"' for t in ts)
        out.append(f'        FieldSpec{{"{k}", {{{joined}}}, {len(ts)}}},')
    return '\n'.join(out)


def schema_block(name, const_name, resource, collection=None):
    collection = collection or name
    flag = 'true' if schema.get(resource, {}).get('object_root') else 'false'
    return (f'inline const EntitySchema {const_name}{{\n'
            f'    "{collection}",\n'
            f'    std::vector<FieldSpec>{{\n'
            f'{spec_lines(resource)}\n'
            f'    }},\n'
            f'    {flag}\n'
            f'}};\n')


blocks = '\n'.join([
    schema_block('posts', 'kPostSchema', 'posts'),
    schema_block('members', 'kMemberSchema', 'members'),
    schema_block('users', 'kUserSchema', 'users'),
    schema_block('newsletters', 'kNewsletterSchema', 'newsletters'),
    schema_block('tiers', 'kTierSchema', 'tiers'),
    schema_block('comments', 'kCommentSchema', 'comments'),
    schema_block('offers', 'kOfferSchema', 'offers'),
    schema_block('notifications', 'kNotificationSchema', 'notifications'),
    schema_block('webhooks', 'kWebhookSchema', 'webhooks'),
    schema_block('emails', 'kEmailSchema', 'emails'),
    schema_block('labels', 'kLabelSchema', 'labels'),
    schema_block('snippets', 'kSnippetSchema', 'snippets'),
    schema_block('roles', 'kRoleSchema', 'roles'),
    schema_block('recommendations', 'kRecommendationSchema', 'recommendations'),
    schema_block('links', 'kLinkSchema', 'links'),
    schema_block('automated_emails', 'kAutomatedEmailSchema', 'automated_emails'),
    schema_block('events', 'kEventSchema', 'events'),
    schema_block('db', 'kDbSchema', 'db'),
    schema_block('bulk', 'kBulkSchema', 'bulk'),
    schema_block('pages', 'kPagesSchema', 'pages'),
    schema_block('member_object', 'kMemberObjectSchema', 'member_object', 'members'),
    schema_block('stats_mrr', 'kStats_mrrSchema', 'stats_mrr', 'stats'),
    schema_block('stats_members', 'kStats_membersSchema', 'stats_members', 'stats'),
    schema_block('stats_subscriptions', 'kStats_subscriptionsSchema', 'stats_subscriptions', 'stats'),

    schema_block('site', 'kSiteSchema', 'site'),
    schema_block('config', 'kConfigSchema', 'config'),
    schema_block('email_previews', 'kEmailPreviewSchema', 'email_previews'),
    schema_block('email_statuses', 'kEmailStatusSchema', 'email_statuses'),
    schema_block('failures', 'kFailuresSchema', 'failures'),
    schema_block('batches', 'kBatchesSchema', 'batches'),
    schema_block('automations', 'kAutomationSchema', 'automations'),
    schema_block('slugs', 'kSlugSchema', 'slugs'),
    schema_block('custom_theme_settings', 'kCustomThemeSettingSchema', 'custom_theme_settings'),
    schema_block('links', 'kLinks2Schema', 'links'),
    schema_block('mentions', 'kMentionSchema', 'mentions'),

    schema_block('settings', 'kSettingSchema', 'settings'),
    schema_block('comment_likes', 'kCommentLikeSchema', 'comment_likes'),
    schema_block('comment_dislikes', 'kCommentDislikeSchema', 'comment_dislikes'),
    schema_block('comment_reports', 'kCommentReportSchema', 'comment_reports'),
])

content = r'''export module blog_testing:ghost_schema;

import std;

export namespace blog::testing {

/** @brief Minimal JSON tree used by the structural snapshot oracle. */
struct JsonNode
{
    enum class Kind { Null, Bool, Number, String, Array, Object };
    Kind kind{Kind::Null};
    std::string scalar{};
    std::vector<JsonNode> array{};
    std::map<std::string, JsonNode> object{};
    [[nodiscard]] bool is(Kind k) const noexcept { return kind == k; }
};

/** @brief Dependency-free recursive-descent JSON parser for response bodies. */
class JsonParser
{
public:
    [[nodiscard]] std::optional<JsonNode> parse(std::string_view text) const
    {
        m_text = text; m_pos = 0;
        skip();
        auto node = value();
        if (!node) return std::nullopt;
        return node;
    }
private:
    void skip() const
    {
        while (m_pos < m_text.size() && std::isspace(static_cast<unsigned char>(m_text[m_pos]))) ++m_pos;
    }
    [[nodiscard]] bool eat(char c) const
    {
        skip();
        if (m_pos < m_text.size() && m_text[m_pos] == c) { ++m_pos; return true; }
        return false;
    }
    [[nodiscard]] std::optional<JsonNode> value() const
    {
        skip();
        if (m_pos >= m_text.size()) return std::nullopt;
        const char c = m_text[m_pos];
        if (c == '{') return object_value();
        if (c == '[') return array_value();
        if (c == '"') {
            JsonNode n; n.kind = JsonNode::Kind::String;
            auto s = string_value();
            if (!s) return std::nullopt;
            n.scalar = *s;
            return n;
        }
        if (c == 't' || c == 'f') {
            JsonNode n; n.kind = JsonNode::Kind::Bool;
            if (m_text.substr(m_pos, 4) == "true") { n.scalar = "true"; m_pos += 4; }
            else if (m_text.substr(m_pos, 5) == "false") { n.scalar = "false"; m_pos += 5; }
            else return std::nullopt;
            return n;
        }
        if (c == 'n') {
            if (m_text.substr(m_pos, 4) != "null") return std::nullopt;
            m_pos += 4;
            return JsonNode{};
        }
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
            JsonNode n; n.kind = JsonNode::Kind::Number;
            const auto start = m_pos;
            if (m_text[m_pos] == '-') ++m_pos;
            while (m_pos < m_text.size() && (std::isdigit(static_cast<unsigned char>(m_text[m_pos])) ||
                   m_text[m_pos] == '.' || m_text[m_pos] == 'e' || m_text[m_pos] == 'E' ||
                   m_text[m_pos] == '+' || m_text[m_pos] == '-')) ++m_pos;
            n.scalar = std::string{m_text.substr(start, m_pos - start)};
            return n;
        }
        return std::nullopt;
    }
    [[nodiscard]] std::optional<std::string> string_value() const
    {
        if (!eat('"')) return std::nullopt;
        std::string out;
        while (m_pos < m_text.size()) {
            const char c = m_text[m_pos++];
            if (c == '"') return out;
            if (c == '\\' && m_pos < m_text.size()) {
                const char e = m_text[m_pos++];
                switch (e) {
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case 'n': out += '\n'; break;
                    case 'r': out += '\r'; break;
                    case 't': out += '\t'; break;
                    default: out += e;
                }
            } else out += c;
        }
        return std::nullopt;
    }
    [[nodiscard]] std::optional<JsonNode> object_value() const
    {
        if (!eat('{')) return std::nullopt;
        JsonNode n; n.kind = JsonNode::Kind::Object;
        if (eat('}')) return n;
        for (;;) {
            auto key = string_value();
            if (!key) return std::nullopt;
            if (!eat(':')) return std::nullopt;
            auto val = value();
            if (!val) return std::nullopt;
            n.object[*key] = *val;
            if (eat('}')) break;
            if (!eat(',')) return std::nullopt;
        }
        return n;
    }
    [[nodiscard]] std::optional<JsonNode> array_value() const
    {
        if (!eat('[')) return std::nullopt;
        JsonNode n; n.kind = JsonNode::Kind::Array;
        if (eat(']')) return n;
        for (;;) {
            auto val = value();
            if (!val) return std::nullopt;
            n.array.push_back(*val);
            if (eat(']')) break;
            if (!eat(',')) return std::nullopt;
        }
        return n;
    }
    mutable std::string_view m_text{};
    mutable std::size_t m_pos{0};
};

/** @brief One Ghost snapshot field and the JSON types its matchers allow. */
struct FieldSpec
{
    std::string_view name;
    std::array<std::string_view, 6> types;
    std::size_t type_count;
};

struct EntitySchema
{
    std::string_view collection;
    std::vector<FieldSpec> fields;
    bool root_object{false};
};

/** @brief Schemas extracted from the Ghost e2e-api snapshot suites. */
''' + blocks + '''
struct ValidationReport
{
    std::size_t checked{0};
    std::size_t passed{0};
    std::vector<std::string> failures{};
};

/** @brief Validate a response body against a Ghost entity schema. */
[[nodiscard]] inline ValidationReport validate(std::string_view body, const EntitySchema& schema)
{
    ValidationReport report;
    const JsonParser parser;
    const auto root = parser.parse(body);
    if (!root || !root->is(JsonNode::Kind::Object)) {
        report.failures.push_back("body is not a JSON object");
        return report;
    }
    const JsonNode* entity_node = nullptr;
    if (schema.root_object) {
        entity_node = &*root;
    } else {
        const auto top = root->object.find(std::string{schema.collection});
        if (top == root->object.end() ||
            (!top->second.is(JsonNode::Kind::Array) && !top->second.is(JsonNode::Kind::Object))) {
            report.failures.push_back("missing collection body: " + std::string{schema.collection});
            return report;
        }
        entity_node = &top->second;
    }
    if (entity_node->is(JsonNode::Kind::Object)) {
        const auto& object = entity_node->object;
        for (const auto& spec : schema.fields) {
            ++report.checked;
            const auto it = object.find(std::string{spec.name});
            if (it == object.end()) {
                report.failures.push_back("missing field: " + std::string{spec.name});
                continue;
            }
            const JsonNode& node = it->second;
            bool ok = false;
            for (std::size_t i = 0; i < spec.type_count; ++i) {
                const auto t = spec.types[i];
                if (t == "any") { ok = true; break; }
                if (t == "null" && node.is(JsonNode::Kind::Null)) { ok = true; break; }
                if (t == "string" && node.is(JsonNode::Kind::String)) { ok = true; break; }
                if (t == "number" && node.is(JsonNode::Kind::Number)) { ok = true; break; }
                if (t == "boolean" && node.is(JsonNode::Kind::Bool)) { ok = true; break; }
                if (t == "object" && node.is(JsonNode::Kind::Object)) { ok = true; break; }
                if (t == "array" && node.is(JsonNode::Kind::Array)) { ok = true; break; }
            }
            if (ok) ++report.passed;
            else report.failures.push_back("wrong type: " + std::string{spec.name});
        }
        return report;
    }
    if (entity_node->array.empty()) return report;
    if (!entity_node->array.front().is(JsonNode::Kind::Object)) {
        report.failures.push_back("collection element is not an object");
        return report;
    }
    const auto& entity = entity_node->array.front().object;
    for (const auto& spec : schema.fields) {
        ++report.checked;
        const auto it = entity.find(std::string{spec.name});
        if (it == entity.end()) {
            report.failures.push_back("missing field: " + std::string{spec.name});
            continue;
        }
        const JsonNode& node = it->second;
        bool ok = false;
        for (std::size_t i = 0; i < spec.type_count; ++i) {
            const auto t = spec.types[i];
            if (t == "any") { ok = true; break; }
            if (t == "null" && node.is(JsonNode::Kind::Null)) { ok = true; break; }
            if (t == "string" && node.is(JsonNode::Kind::String)) { ok = true; break; }
            if (t == "number" && node.is(JsonNode::Kind::Number)) { ok = true; break; }
            if (t == "boolean" && node.is(JsonNode::Kind::Bool)) { ok = true; break; }
            if (t == "object" && node.is(JsonNode::Kind::Object)) { ok = true; break; }
            if (t == "array" && node.is(JsonNode::Kind::Array)) { ok = true; break; }
        }
        if (ok) ++report.passed;
        else report.failures.push_back("wrong type: " + std::string{spec.name});
    }
    return report;
}

} // namespace blog::testing
'''
pathlib.Path('include/blog/testing/ghost_schema.cppm').write_text(content)
print('generated')
