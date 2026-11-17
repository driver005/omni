export module core_activitypub:inbox;

import std;
import :activity;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::activitypub {

/** @brief Per-actor inbox: ordered list of received activities. */
class Inbox
{
public:
    Inbox() = default;

    explicit Inbox(std::string owner) :
        m_owner{std::move(owner)}
    {
    }

    [[nodiscard]] const std::string& owner() const noexcept
    {
        return m_owner;
    }

    void receive(Activity activity)
    {
        m_activities.push_back(std::move(activity));
    }

    [[nodiscard]] const std::vector<Activity>& activities() const noexcept
    {
        return m_activities;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_activities.size();
    }

private:
    std::string m_owner{};
    std::vector<Activity> m_activities{};
};

} // namespace core::activitypub

#ifdef CONGELADO_TEST
namespace core::activitypub::tests {
using namespace boost::ut;

suite<"core_activitypub_inbox"> core_activitypub_inbox_suite = []
{
    "receives activities in order"_test = []
    {
        Inbox inbox{"https://x/u/ada"};
        inbox.receive(Activity{"1", ActivityType::Create, "https://x/u/bob", "o1"});
        inbox.receive(Activity{"2", ActivityType::Like, "https://x/u/bob", "o1"});
        expect(inbox.size() == 2);
        expect(inbox.activities()[0].id() == "1");
    };
};

} // namespace core::activitypub::tests
#endif
