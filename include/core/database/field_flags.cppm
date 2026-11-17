export module core_database:field_flags;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

/** @brief Compile-time bitmask of constraints attached to a reflected column. */
enum class FieldFlags : std::uint8_t
{
    NONE = 0,
    PRIMARY_KEY = 1,
    NOT_NULL = 2,
    UNIQUE = 4
};

[[nodiscard]] constexpr FieldFlags operator|(FieldFlags a, FieldFlags b) noexcept
{
    return static_cast<FieldFlags>(
        static_cast<std::uint8_t>(a) | static_cast<std::uint8_t>(b)
    );
}

[[nodiscard]] constexpr FieldFlags operator&(FieldFlags a, FieldFlags b) noexcept
{
    return static_cast<FieldFlags>(
        static_cast<std::uint8_t>(a) & static_cast<std::uint8_t>(b)
    );
}

/** @brief Instance functor testing a FieldFlags bitmask. */
class HasFlag
{
public:
    [[nodiscard]] constexpr bool operator()(FieldFlags value, FieldFlags flag) const noexcept
    {
        return (value & flag) != FieldFlags::NONE;
    }
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {
using namespace boost::ut;

suite<"core_db_field_flags"> core_db_field_flags_suite = []
{
    "flags compose and test"_test = []
    {
        constexpr FieldFlags flags = FieldFlags::PRIMARY_KEY | FieldFlags::NOT_NULL;
        expect(HasFlag{}(flags, FieldFlags::PRIMARY_KEY));
        expect(HasFlag{}(flags, FieldFlags::NOT_NULL));
        expect(!HasFlag{}(flags, FieldFlags::UNIQUE));
        expect((flags & FieldFlags::UNIQUE) == FieldFlags::NONE);
    };
};

} // namespace core::db::tests
#endif
