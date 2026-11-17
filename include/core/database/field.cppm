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
