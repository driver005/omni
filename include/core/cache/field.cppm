export module core_cache:field;

import std;
import :string_literal;
import :member_pointer_traits;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache {

/** @brief What a reflected member contributes to a cache entry. */
enum class FieldRole : std::uint8_t
{
    /** @brief Part of the composite cache key. */
    KEY,
    /** @brief Stored as a tag for tag-based queries. */
    TAG,
    /** @brief Stored as a plain attribute. */
    PAYLOAD
};

} // namespace core::cache

export namespace core::cache::reflect {

/** @brief One reflected member: its name, member pointer and role. Compile-time only. */
template<StringLiteral Name, auto MemberPointer, FieldRole Role = FieldRole::PAYLOAD>
struct CacheField
{
    using traits = MemberPointerTraits<decltype(MemberPointer)>;
    using entity_type = typename traits::class_t;
    using value_type = typename traits::value_t;

    static constexpr auto name = Name;
    static constexpr auto member = MemberPointer;
    static constexpr FieldRole role = Role;
    static constexpr bool is_key = role == FieldRole::KEY;
    static constexpr bool is_tag = role == FieldRole::TAG;
};

} // namespace core::cache::reflect

#ifdef CONGELADO_TEST
namespace core::cache::reflect::tests {
using namespace boost::ut;

struct FieldSample
{
    std::int64_t id{};
};

suite<"core_cache_cache_field"> core_cache_cache_field_suite = []
{
    "captures name, member and role at compile time"_test = []
    {
        using F = CacheField<"id", &FieldSample::id, FieldRole::KEY>;
        static_assert(std::same_as<F::entity_type, FieldSample>);
        static_assert(std::same_as<F::value_type, std::int64_t>);
        static_assert(F::is_key);
        expect(F::name.view() == "id");
    };
};

} // namespace core::cache::reflect::tests
#endif
