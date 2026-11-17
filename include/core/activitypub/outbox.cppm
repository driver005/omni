export module core_activitypub:outbox;

import std;
import :activity;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::activitypub {

/** @brief Per-actor outbox: ordered list of published activities. */
class Outbox
{
public:
    Outbox() = default;

    explicit Outbox(std::string owner) :
        m_owner{std::move(owner)}
    {
    }

    [[nodiscard]] const std::string& owner() const noexcept
    {
        return m_owner;
    }

    void publish(Activity activity)
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

suite<"core_activitypub_outbox"> core_activitypub_outbox_suite = []
{
    "publishes activities in order"_test = []
    {
        Outbox outbox{"https://x/u/ada"};
        outbox.publish(Activity{"1", ActivityType::Create, "https://x/u/ada", "o1"});
        expect(outbox.size() == 1);
        expect(outbox.activities()[0].id() == "1");
    };
};

} // namespace core::activitypub::tests
#endif
