export module core_activitypub:builder;

import std;
import :activitypub;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::activitypub {

/** @brief Fluent entry point — sole public way to construct an ActivityPub handle. */
class ActivityPubBuilder
{
public:
    ActivityPubBuilder& option(std::string key, std::string value)
    {
        m_options.insert_or_assign(std::move(key), std::move(value));
        return *this;
    }

    [[nodiscard]] ActivityPub build() const
    {
        return ActivityPub{};
    }

private:
    std::map<std::string, std::string> m_options{};
};

} // namespace core::activitypub

#ifdef CONGELADO_TEST
namespace core::activitypub::tests {
using namespace boost::ut;

suite<"core_activitypub_activitypub_builder"> core_activitypub_activitypub_builder_suite = []
{
    "builds an ActivityPub handle"_test = []
    {
        const ActivityPub ap = ActivityPubBuilder{}.option("domain", "x.example").build();
        expect(ap.inbox_size() == 0);
    };
};

} // namespace core::activitypub::tests
#endif
