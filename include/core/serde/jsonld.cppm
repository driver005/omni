export module core_serde:jsonld;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace serde::jsonld {

/** @brief A JSON-LD `@context` value: a URL reference or an inline map. */
class JsonLdContext
{
public:
    JsonLdContext() = default;

    explicit JsonLdContext(std::string reference) :
        m_reference{std::move(reference)}
    {
    }

    [[nodiscard]] const std::string& reference() const noexcept
    {
        return m_reference;
    }

    [[nodiscard]] bool valid() const noexcept
    {
        return !m_reference.empty();
    }

private:
    std::string m_reference{};
};

/** @brief A minimal ActivityStreams-style JSON-LD document value. */
class JsonLdDocument
{
public:
    JsonLdDocument() = default;

    JsonLdDocument(JsonLdContext context, std::string type, std::string id) :
        m_context{std::move(context)},
        m_type{std::move(type)},
        m_id{std::move(id)}
    {
    }

    [[nodiscard]] const JsonLdContext& context() const noexcept
    {
        return m_context;
    }

    [[nodiscard]] const std::string& type() const noexcept
    {
        return m_type;
    }

    [[nodiscard]] const std::string& id() const noexcept
    {
        return m_id;
    }

    JsonLdDocument& set(std::string key, std::string value)
    {
        m_fields.insert_or_assign(std::move(key), std::move(value));
        return *this;
    }

    [[nodiscard]] std::optional<std::string> field(std::string_view key) const
    {
        const auto it = m_fields.find(std::string{key});
        return it == m_fields.end() ? std::nullopt : std::optional<std::string>{it->second};
    }

    [[nodiscard]] std::size_t field_count() const noexcept
    {
        return m_fields.size();
    }

    [[nodiscard]] bool valid() const noexcept
    {
        return m_context.valid() && !m_type.empty();
    }

private:
    JsonLdContext m_context{};
    std::string m_type{};
    std::string m_id{};
    std::map<std::string, std::string> m_fields{};
};

} // namespace serde::jsonld

#ifdef CONGELADO_TEST
namespace serde::jsonld::tests {
using namespace boost::ut;

suite<"serde_jsonld"> serde_jsonld_suite = []
{
    "builds a JSON-LD document"_test = []
    {
        JsonLdDocument document{
            JsonLdContext{"https://www.w3.org/ns/activitystreams"},
            "Note",
            "https://x/note/1"
        };
        document.set("content", "hello");
        expect(document.valid());
        expect(document.type() == "Note");
        expect(document.field("content") == std::string{"hello"});
        expect(document.field_count() == 1);
    };
};

} // namespace serde::jsonld::tests
#endif
