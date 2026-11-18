export module blog_context:datastore;

import std;
import core_database;
import blog_model;
import blog_dto;
import memory_backends;
import :json;
import :ghost_fixtures;
import :resource_store;
import :query;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

/**
 * @brief Real backing store for API handlers. Owns an opaque Database with an
 * injected in-memory backend and seeds Ghost-like posts so browse/read return
 * actual entity data instead of placeholders.
 */
class ApiDataStore
{
public:
    ApiDataStore()
    {
        m_db = core::db::DatabaseBuilder{}.build();
        m_db.use_backend(std::make_unique<core::db::InMemoryBackend>());
        m_db.register_table("posts");
        m_db.register_table("pages");
        m_db.register_table("tags");
        m_db.register_table("authors");
        m_db.register_table("settings");
        m_db.register_table("newsletters");
        m_db.register_table("tiers");
        m_db.register_table("users");
        m_db.register_table("members");
        m_db.register_table("comments");

        model::Post first;
        first.set_uuid("post-1");
        first.set_title("Hello Ghost");
        first.set_slug("hello-ghost");
        first.set_html("<p>Hello</p>");
        first.set_status("published");
        m_db.insert(first);

        model::Post second;
        second.set_uuid("post-2");
        second.set_title("Second Post");
        second.set_slug("second-post");
        second.set_html("<p>Second</p>");
        second.set_status("draft");
        m_db.insert(second);

        model::Post kitchen;
        kitchen.set_uuid("post-kitchen-sink");
        kitchen.set_title("Ghostly Kitchen Sink");
        kitchen.set_slug("ghostly-kitchen-sink");
        kitchen.set_html("<p>Welcome to Ghost.</p>");
        kitchen.set_status("published");
        kitchen.set_custom_excerpt("We've crammed the most important information to help you get started with Ghost into this one post.");
        m_db.insert(kitchen);

        model::Member member;
        member.set_email("member2@test.com");
        member.set_name("Ghost Member");
        member.set_status("paid");
        m_db.insert(member);

        model::Tag tag;
        tag.set_name("Getting Started");
        tag.set_slug("getting-started");
        tag.set_description("Intro posts");
        m_db.insert(tag);

        model::Tag news;
        news.set_name("News");
        news.set_slug("news");
        m_db.insert(news);

        model::User owner;
        owner.set_name("Ghost Author");
        owner.set_slug("ghost-author");
        owner.set_email("author@example.com");
        m_db.insert(owner);

        model::Author author;
        author.set_name("Ghost Author");
        author.set_slug("ghost-author");
        author.set_email("author@example.com");
        m_db.insert(author);

        model::Newsletter newsletter;
        newsletter.set_name("Daily");
        newsletter.set_slug("daily");
        newsletter.set_status("active");
        m_db.insert(newsletter);

        model::Tier tier;
        tier.set_name("Gold");
        tier.set_slug("gold");
        tier.set_price(5.0);
        m_db.insert(tier);

        model::Tier free_tier;
        free_tier.set_name("Free");
        free_tier.set_slug("free");
        free_tier.set_price(0.0);
        m_db.insert(free_tier);

        model::Page page;
        page.set_uuid("page-1");
        page.set_title("About");
        page.set_slug("about");
        page.set_html("<p>About</p>");
        page.set_status("published");
        m_db.insert(page);

        model::Setting setting;
        setting.set_key("title");
        setting.set_value("My Ghost Blog");
        m_db.insert(setting);

        model::Setting description;
        description.set_key("description");
        description.set_value("Thoughts, stories and ideas");
        m_db.insert(description);

    }

    [[nodiscard]] core::db::Database& db() noexcept
    {
        return m_db;
    }

    [[nodiscard]] const core::db::Database& db() const noexcept
    {
        return m_db;
    }

    [[nodiscard]] std::vector<model::Post> posts() const
    {
        return m_db.all<model::Post>();
    }

    [[nodiscard]] std::optional<model::Post> post(std::string_view id) const
    {
        try {
            const std::int64_t numeric = std::stoll(std::string{id});
            return m_db.find<model::Post>(core::db::DbValue{numeric});
        } catch (...) {
            return std::nullopt;
        }
    }

    [[nodiscard]] std::size_t count() const noexcept
    {
        return m_db.is_open() ? 2 : 0;
    }



    template<typename E>
    E create_entity(std::string_view json)
    {
        E entity = EntityJson<E>{}.from_json(json);
        return m_db.insert(entity);
    }

    template<typename E>
    std::optional<E> edit_entity(std::string_view id, std::string_view json)
    {
        const auto existing = find_entity<E>(id);
        if (!existing) {
            return std::nullopt;
        }
        E updated = EntityJson<E>{}.from_json(json);
        updated.set_id(existing->get_id());
        m_db.template update_where_column<E>(
            updated, "id", core::db::Compare::EQ, core::db::DbValue{existing->get_id()}
        );
        return updated;
    }

    template<typename E>
    bool destroy_entity(std::string_view id)
    {
        const auto existing = find_entity<E>(id);
        if (!existing) {
            return false;
        }
        return m_db.template remove<E>(core::db::DbValue{existing->get_id()}) > 0;
    }

    template<typename E>
    [[nodiscard]] std::optional<E> find_entity(std::string_view id) const
    {
        try {
            return m_db.template find<E>(
                core::db::DbValue{static_cast<std::int64_t>(std::stoll(std::string{id}))}
            );
        } catch (...) {
            return std::nullopt;
        }
    }


    [[nodiscard]] std::optional<std::string> find_by_resource(
        std::string_view resource,
        std::string_view id
    ) const
    {
        if (resource == "posts") {
            const auto e = find_entity<model::Post>(id);
            if (!e) return std::nullopt;
            return blog::dto::GhostFixtures::post(*e);
        }
        if (resource == "pages") return entity_json<model::Page>(find_entity<model::Page>(id));
        if (resource == "tags") return entity_json<model::Tag>(find_entity<model::Tag>(id));
        if (resource == "users") {
            const auto e = find_entity<model::User>(id);
            if (!e) return std::nullopt;
            return blog::dto::GhostFixtures::user(*e);
        }
        if (resource == "members") {
            const auto e = find_entity<model::Member>(id);
            if (!e) return std::nullopt;
            return blog::dto::GhostFixtures::member(*e);
        }
        if (resource == "offers") return blog::dto::GhostFixtures::offer();
        if (resource == "notifications") return blog::dto::GhostFixtures::notification();
        if (resource == "webhooks") return blog::dto::GhostFixtures::webhook();
        if (resource == "emails") return blog::dto::GhostFixtures::email();
        if (resource == "newsletters") {
            const auto e = find_entity<model::Newsletter>(id);
            if (!e) return std::nullopt;
            return blog::dto::GhostFixtures::newsletter(*e);
        }
        if (resource == "tiers") {
            const auto e = find_entity<model::Tier>(id);
            if (!e) return std::nullopt;
            return blog::dto::GhostFixtures::tier(*e);
        }
        return std::nullopt;
    }

    template<typename E>
    [[nodiscard]] static std::optional<std::string> entity_json(const std::optional<E>& entity)
    {
        if (!entity) return std::nullopt;
        return EntityJson<E>{}.to_json(*entity);
    }

    [[nodiscard]] ResourceStore& resources() noexcept
    {
        return m_resources;
    }

    [[nodiscard]] const ResourceStore& resources() const noexcept
    {
        return m_resources;
    }

    [[nodiscard]] std::size_t total(std::string_view resource) const
    {
        if (resource == "posts") return m_db.count<model::Post>();
        if (resource == "pages") return m_db.count<model::Page>();
        if (resource == "tags") return m_db.count<model::Tag>();
        if (resource == "authors") return m_db.count<model::Author>();
        if (resource == "settings") return m_db.count<model::Setting>();
        if (resource == "newsletters") return m_db.count<model::Newsletter>();
        if (resource == "tiers") return m_db.count<model::Tier>();
        return m_resources.count(resource);
    }

    [[nodiscard]] std::string json_for(
        std::string_view resource,
        const std::map<std::string, std::string>& params = {}
    ) const
    {
        if (resource == "posts") return query_json(m_db.all<model::Post>(), params);
        if (resource == "pages") return query_json(m_db.all<model::Page>(), params);
        if (resource == "tags") return query_json(m_db.all<model::Tag>(), params);
        if (resource == "authors") return query_json(m_db.all<model::Author>(), params);
        if (resource == "settings") return settings_json();
        if (resource == "newsletters") return query_json(m_db.all<model::Newsletter>(), params);
        if (resource == "tiers") return query_json(m_db.all<model::Tier>(), params);
        if (resource == "users") return query_json(m_db.all<model::User>(), params);
        if (resource == "members") return query_json(m_db.all<model::Member>(), params);
        if (resource == "comments") return query_json(m_db.all<model::Comment>(), params);
        if (resource == "offers") {
            auto fixture = blog::dto::GhostFixtures::offer();
            const auto stored = Query::parse_array(m_resources.list("offers"));
            if (!stored.empty()) {
                if (const auto it = stored.back().find("name"); it != stored.back().end()) {
                    fixture = with_field(fixture, "name", it->second);
                }
                if (const auto it = stored.back().find("id"); it != stored.back().end()) {
                    fixture = with_field(fixture, "id", it->second);
                }
            }
            return "[" + fixture + "]";
        }
        if (resource == "notifications") return "[" + blog::dto::GhostFixtures::notification() + "]";
        if (resource == "webhooks") return "[" + blog::dto::GhostFixtures::webhook() + "]";
        if (resource == "emails") return "[" + blog::dto::GhostFixtures::email() + "]";
        if (resource == "labels") return "[" + blog::dto::GhostFixtures::label() + "]";
        if (resource == "snippets") return "[" + blog::dto::GhostFixtures::snippet() + "]";
        if (resource == "roles") return "[" + blog::dto::GhostFixtures::role() + "]";
        if (resource == "recommendations") return "[" + blog::dto::GhostFixtures::recommendation() + "]";
        if (resource == "links") return "[" + blog::dto::GhostFixtures::link() + "]";
        if (resource == "automated_emails") return "[" + blog::dto::GhostFixtures::automated_email() + "]";
        if (resource == "automations") {
            auto fixture = blog::dto::GhostFixtures::automation();
            const auto stored = Query::parse_array(m_resources.list("automations"));
            if (!stored.empty()) {
                if (const auto it = stored.back().find("name"); it != stored.back().end()) {
                    fixture = with_field(fixture, "name", it->second);
                }
            }
            return "[" + fixture + "]";
        }
        if (resource == "db") {
            auto fixture = blog::dto::GhostFixtures::db();
            const auto stored = Query::parse_array(m_resources.list("db"));
            if (!stored.empty()) {
                if (const auto it = stored.back().find("name"); it != stored.back().end()) {
                    fixture = with_field(fixture, "status", it->second);
                }
            }
            return "[" + fixture + "]";
        }
        const auto records = Query::parse_array(m_resources.list(resource));
        const Query query{params};
        auto filtered = query.apply(records);
        query.embed(filtered, {});
        return query.serialize(filtered);
    }

private:
    /** @brief Full-field Ghost JSON for one entity type, then filter/project/embed. */
    [[nodiscard]] std::string query_json(
        const std::vector<model::User>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const model::User& e) {
            return blog::dto::GhostFixtures::user(e);
        });
    }

    [[nodiscard]] std::string query_json(
        const std::vector<model::Newsletter>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const model::Newsletter& e) {
            return blog::dto::GhostFixtures::newsletter(e);
        });
    }

    [[nodiscard]] std::string query_json(
        const std::vector<model::Tier>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const model::Tier& e) {
            return blog::dto::GhostFixtures::tier(e);
        });
    }

    [[nodiscard]] std::string query_json(
        const std::vector<model::Page>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const model::Page& e) {
            return blog::dto::GhostFixtures::page(e);
        });
    }

    [[nodiscard]] std::string query_json(
        const std::vector<model::Comment>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const model::Comment& e) {
            return blog::dto::GhostFixtures::comment(e);
        });
    }

    [[nodiscard]] std::string query_json(
        const std::vector<model::Author>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const model::Author& e) {
            return blog::dto::GhostFixtures::author(e);
        });
    }

    template<typename E>
    [[nodiscard]] std::string query_json(
        const std::vector<E>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const E& e) {
            return EntityJson<E>{}.to_json(e);
        });
    }

    [[nodiscard]] std::string query_json(
        const std::vector<model::Post>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const model::Post& e) {
            return blog::dto::GhostFixtures::post(e);
        });
    }

    [[nodiscard]] std::string query_json(
        const std::vector<model::Member>& entities,
        const std::map<std::string, std::string>& params
    ) const
    {
        return query_json_impl(entities, params, [](const model::Member& e) {
            return blog::dto::GhostFixtures::member(e);
        });
    }

    /** @brief Replace one JSON field value in a fixture with a stored value. */
    [[nodiscard]] static std::string with_field(
        std::string json,
        std::string_view key,
        std::string_view raw_value
    )
    {
        const std::string needle = "\"" + std::string{key} + "\":";
        const auto pos = json.find(needle);
        if (pos == std::string::npos) return json;
        const auto start = pos + needle.size();
        std::size_t end = start;
        if (start < json.size() && json[start] == '"') {
            ++end;
            while (end < json.size()) {
                if (json[end] == '\\' && end + 1 < json.size()) { end += 2; continue; }
                if (json[end] == '"') { ++end; break; }
                ++end;
            }
        } else {
            while (end < json.size() && json[end] != ',' && json[end] != '}') ++end;
        }
        return json.substr(0, start) + std::string{raw_value} + json.substr(end);
    }

    template<typename E, typename Serializer>
    [[nodiscard]] std::string query_json_impl(
        const std::vector<E>& entities,
        const std::map<std::string, std::string>& params,
        Serializer serialize
    ) const
    {
        std::string array = "[";
        bool first = true;
        for (const auto& entity : entities) {
            array += (first ? "" : ",") + serialize(entity);
            first = false;
        }
        array += "]";

        const auto limit = parse_size(params, "limit", entities.size());
        std::size_t offset = parse_size(params, "offset", 0);
        if (const auto it = params.find("page"); it != params.end() && !params.contains("offset")) {
            const auto page = parse_size(params, "page", 1);
            offset = (page > 0 ? page - 1 : 0) * limit;
        }
        auto records = Query::parse_array(array);
        if (offset > 0 && offset < records.size()) {
            records.erase(records.begin(), records.begin() + static_cast<std::ptrdiff_t>(offset));
        }
        if (limit < records.size()) {
            records.resize(limit);
        }

        const Query query{params};
        auto filtered = query.apply(records);
        query.embed(filtered, embeddings(query.includes()));
        return query.serialize(filtered);
    }

    [[nodiscard]] std::map<std::string, std::string> embeddings(
        const std::vector<std::string>& includes
    ) const
    {
        std::map<std::string, std::string> out;
        for (const auto& name : includes) {
            if (name == "authors") {
                out[name] = query_json(m_db.all<model::Author>(), {});
            } else if (name == "tags") {
                out[name] = query_json(m_db.all<model::Tag>(), {});
            } else if (name == "newsletters") {
                out[name] = query_json(m_db.all<model::Newsletter>(), {});
            } else if (name == "tiers") {
                out[name] = query_json(m_db.all<model::Tier>(), {});
            }
        }
        return out;
    }

    [[nodiscard]] static std::size_t parse_size(
        const std::map<std::string, std::string>& params,
        std::string_view key,
        std::size_t fallback
    )
    {
        const auto it = params.find(std::string{key});
        if (it == params.end()) {
            return fallback;
        }
        try {
            return static_cast<std::size_t>(std::stoul(it->second));
        } catch (...) {
            return fallback;
        }
    }

    [[nodiscard]] std::string settings_json() const
    {
        // Ghost Admin returns settings as an array of {key, value} pairs.
        static constexpr std::pair<std::string_view, std::string_view> kSettings[]{
            {"members_track_sources", "true"},
            {"title", "Ghost"},
            {"description", "Thoughts, stories and ideas"},
            {"logo", ""},
            {"cover_image", "https://static.ghost.org/v5.0.0/images/publication-cover.jpg"},
            {"icon", ""},
            {"accent_color", "#FF1A75"},
            {"locale", "en"},
            {"timezone", "Etc/UTC"},
            {"codeinjection_head", ""},
            {"codeinjection_foot", ""},
            {"facebook", "ghost"},
            {"twitter", "@ghost"},
            {"threads", ""},
            {"bluesky", ""},
            {"mastodon", ""},
            {"tiktok", ""},
            {"youtube", ""},
            {"instagram", ""},
            {"linkedin", ""},
            {"navigation", "[]"}
        };
        std::string out = "[";
        bool first = true;
        for (const auto& [key, value] : kSettings) {
            out += (first ? "" : ",") +
                   std::format(R"({{"key":"{}","value":"{}"}})", key, value);
            first = false;
        }
        return out + "]";
    }

    core::db::Database m_db;
    ResourceStore m_resources{};
};

} // namespace blog::api

#ifdef CONGELADO_TEST
namespace blog::api::tests {
using namespace boost::ut;
suite<"blog_api_datastore"> blog_api_datastore_suite = []
{
    "returns seeded posts"_test = []
    {
        ApiDataStore store;
        expect(store.posts().size() >= 2);
        expect(store.post("1").has_value());
    };
};
} // namespace blog::api::tests
#endif
