export module core_cache:query;

import std;
import :compare;
import :query_spec;
import :predicate;
import :entity_meta;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache {

/**
 * @brief Fluent, entity-typed cache query builder.
 *
 * Member pointers are checked at compile time. The produced CacheQuerySpec is
 * plain data — no backend handle ever reaches the caller.
 */
template<reflect::ReflectedCacheEntity E>
class CacheQuery
{
public:
    CacheQuery() :
        m_spec{.prefix = std::string{reflect::CacheEntityMeta<E>::prefix} + ":v1"}
    {
    }

    /** @brief Adds a typed predicate on a reflected member. */
    template<auto Member>
    CacheQuery& where(CacheCompare comparison, std::string value)
    {
        static_assert(
            reflect::is_entity_member_v<E, Member>,
            "where() member is not reflected on this entity"
        );
        m_spec.predicates.push_back(CachePredicate{
            .field = std::string{reflect::name_of<E, Member>()},
            .comparison = comparison,
            .value = std::move(value),
        });
        return *this;
    }

    /** @brief Equality shorthand for a reflected member. */
    template<auto Member>
    CacheQuery& where_eq(std::string value)
    {
        return where<Member>(CacheCompare::EQ, std::move(value));
    }

    /** @brief Requires an entry to carry a tag. */
    CacheQuery& with_tag(std::string tag)
    {
        m_spec.required_tags.push_back(std::move(tag));
        return *this;
    }

    /** @brief Overrides the key prefix (defaults to the entity's namespace). */
    CacheQuery& prefix(std::string value)
    {
        m_spec.prefix = std::move(value);
        return *this;
    }

    CacheQuery& limit(std::size_t count)
    {
        m_spec.limit_count = count;
        return *this;
    }

    [[nodiscard]] const CacheQuerySpec& spec() const noexcept
    {
        return m_spec;
    }

private:
    CacheQuerySpec m_spec{};
};

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {

struct QuerySample
{
    std::int64_t id{};
    std::string name{};
    bool active{};
};
CORE_CACHE_ENTITY(QuerySample, "users",
    CORE_CACHE_FIELD("id", &QuerySample::id, FieldRole::KEY),
    CORE_CACHE_FIELD("name", &QuerySample::name, FieldRole::TAG),
    CORE_CACHE_FIELD("active", &QuerySample::active));

using namespace boost::ut;

suite<"core_cache_query"> core_cache_query_suite = []
{
    "composes typed predicates, tags and limits"_test = []
    {
        CacheQuery<QuerySample> query;
        query.where<&QuerySample::id>(CacheCompare::GE, "18")
            .with_tag("admin")
            .limit(5);
        expect(query.spec().prefix == "users:v1");
        expect(query.spec().predicates.size() == 1);
        expect(query.spec().predicates[0].field == "id");
        expect(query.spec().required_tags.size() == 1);
        expect(query.spec().limit_count == 5);
    };
};

} // namespace core::cache::tests
#endif
