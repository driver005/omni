export module core_database:field;

import std;
import :string_literal;
import :column_type;
import :field_flags;
import :member_pointer_traits;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db::reflect {

/**
 * @brief One reflected field: column name, C++ member pointer, SQL kind and
 * flags. Every member is a compile-time constant.
 */
template<
    StringLiteral Name,
    auto MemberPointer,
    ColumnType Type,
    FieldFlags Flags = FieldFlags::NONE>
struct Field
{
    using traits = MemberPointerTraits<decltype(MemberPointer)>;
    using entity_type = typename traits::class_t;
    using value_type = typename traits::value_t;

    static constexpr auto name = Name;
    static constexpr auto member = MemberPointer;
    static constexpr ColumnType type = Type;
    static constexpr FieldFlags flags = Flags;

    static constexpr bool primary_key = HasFlag{}(Flags, FieldFlags::PRIMARY_KEY);
    static constexpr bool not_null = HasFlag{}(Flags, FieldFlags::NOT_NULL) || primary_key;
    static constexpr bool unique = HasFlag{}(Flags, FieldFlags::UNIQUE);
};


template<typename>
struct SetterValue;

template<typename C, typename V>
struct SetterValue<void (C::*)(V)>
{
    using type = std::remove_cvref_t<V>;
};

/**
 * @brief Reflection descriptor backed by serde getters/setters instead of a
 * data-member pointer. Lets DB code consume serde-described models directly.
 */
template<
    auto Name,
    auto Getter,
    auto Setter,
    ColumnType Type,
    bool PrimaryKey = false,
    bool NotNull = false,
    bool Unique = false>
struct SerdeField
{
    using traits = MemberPointerTraits<decltype(Getter)>;
    using entity_type = typename traits::class_t;
    using value_type = typename SetterValue<decltype(Setter)>::type;

    static constexpr auto name = Name.string_view();
    static constexpr auto getter = Getter;
    static constexpr auto setter = Setter;
    static constexpr ColumnType type = Type;
    static constexpr bool primary_key = PrimaryKey;
    static constexpr bool not_null = NotNull || PrimaryKey;
    static constexpr bool unique = Unique;
};

/** @brief Reflection descriptor backed by a public data member from serde DataFieldDesc. */
template<auto Name, auto Member, ColumnType Type, bool PrimaryKey = false>
struct SerdeDataField
{
    using traits = MemberPointerTraits<decltype(Member)>;
    using entity_type = typename traits::class_t;
    using value_type = std::remove_cvref_t<typename traits::value_t>;

    static constexpr auto name = Name.string_view();
    static constexpr auto member = Member;
    static constexpr ColumnType type = Type;
    static constexpr bool primary_key = PrimaryKey;
    static constexpr bool not_null = PrimaryKey;
    static constexpr bool unique = false;
};

/** @brief Write a field described by either member pointer or serde setter. */
template<typename Entity, typename FieldDesc, typename Value>
void set_field(Entity& entity, FieldDesc, const Value& value)
{
    if constexpr (requires { entity.*FieldDesc::member; }) {
        entity.*FieldDesc::member = value;
    } else {
        (entity.*FieldDesc::setter)(value);
    }
}

/** @brief Read a field described by either member pointer or serde getter. */
template<typename Entity, typename FieldDesc>
[[nodiscard]] decltype(auto) get_field(const Entity& entity, FieldDesc)
{
    if constexpr (requires { entity.*FieldDesc::member; }) {
        return (entity.*FieldDesc::member);
    } else {
        return (entity.*FieldDesc::getter)();
    }
}

} // namespace core::db::reflect

#ifdef CONGELADO_TEST
namespace core::db::reflect::tests {
using namespace boost::ut;

struct FieldSample
{
    std::int64_t id{};
};

suite<"core_db_field"> core_db_field_suite = []
{
    "captures member, name, type and flags at compile time"_test = []
    {
        using F = Field<"id", &FieldSample::id, ColumnType::BIGINT, FieldFlags::PRIMARY_KEY>;
        static_assert(std::same_as<F::entity_type, FieldSample>);
        static_assert(std::same_as<F::value_type, std::int64_t>);
        static_assert(F::primary_key);
        static_assert(F::not_null);
        expect(F::name.view() == "id");
        expect(F::type == ColumnType::BIGINT);
    };
};

} // namespace core::db::reflect::tests
#endif
