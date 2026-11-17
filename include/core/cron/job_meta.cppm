export module core_cron:job_meta;

import std;
import :string_literal;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cron::reflect {

/**
 * @brief Compile-time metadata for a scheduled job class, discovered by ADL on
 * `cron_reflect_meta(E*)`, emitted by CORE_CRON_JOB in the class's namespace.
 */
template<typename E>
struct CronJobMeta
{
private:
    using MetaTuple = decltype(cron_reflect_meta(static_cast<E*>(nullptr)));

public:
    static constexpr std::string_view name =
        std::get<0>(cron_reflect_meta(static_cast<E*>(nullptr)));
    static constexpr std::string_view expression =
        std::get<1>(cron_reflect_meta(static_cast<E*>(nullptr)));
    using entity_type = E;
};

template<typename E>
concept ReflectedCronJob = requires {
    cron_reflect_meta(static_cast<E*>(nullptr));
} && requires {
    { CronJobMeta<E>::name } -> std::convertible_to<std::string_view>;
    { CronJobMeta<E>::expression } -> std::convertible_to<std::string_view>;
    typename CronJobMeta<E>::entity_type;
} && std::same_as<typename CronJobMeta<E>::entity_type, E>;

} // namespace core::cron::reflect

/**
 * @brief Declares a class as a reflected cron job with a compile-time name and
 * expression. The metadata is validated at compile time by the static_assert.
 */
#define CORE_CRON_JOB(Type, Name, Expression)                                    \
    consteval auto cron_reflect_meta(Type*) {                                    \
        return ::std::make_tuple(                                                \
            ::std::string_view{Name},                                            \
            ::std::string_view{Expression}                                       \
        );                                                                       \
    }                                                                            \
    static_assert(                                                               \
        ::core::cron::reflect::ReflectedCronJob<Type>,                          \
        "CORE_CRON_JOB: " #Type " failed entity validation"                     \
    )

#ifdef CONGELADO_TEST
namespace core::cron::reflect::tests {

struct NightlyJob
{
     void run() {}
};
CORE_CRON_JOB(NightlyJob, "nightly-backup", "0 2 * * *");

using namespace boost::ut;

suite<"core_cron_job_meta"> core_cron_job_meta_suite = []
{
    "ADL metadata exposes name and expression at compile time"_test = []
    {
        static_assert(ReflectedCronJob<NightlyJob>);
        static_assert(CronJobMeta<NightlyJob>::name == "nightly-backup");
        static_assert(CronJobMeta<NightlyJob>::expression == "0 2 * * *");
        expect(CronJobMeta<NightlyJob>::name == "nightly-backup");
    };
};

} // namespace core::cron::reflect::tests
#endif
