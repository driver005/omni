export module core_database:member_pointer_traits;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db::reflect {

/** @brief Decomposes `V C::*` into its owner and value types. */
template<typename>
struct MemberPointerTraits;

template<typename C, typename V>
struct MemberPointerTraits<V C::*>
{
    using class_t = C;
    using value_t = std::remove_cv_t<V>;
};

} // namespace core::db::reflect

#ifdef CONGELADO_TEST
namespace core::db::reflect::tests {
using namespace boost::ut;

struct TraitsSample
{
    std::string label{};
};

suite<"core_db_member_pointer_traits"> core_db_member_pointer_traits_suite = []
{
    "extracts owner and value type"_test = []
    {
        using T = MemberPointerTraits<decltype(&TraitsSample::label)>;
        static_assert(std::same_as<T::class_t, TraitsSample>);
        static_assert(std::same_as<T::value_t, std::string>);
        expect(true);
    };
};

} // namespace core::db::reflect::tests
#endif
