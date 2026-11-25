export module core_activitypub:activity;

import std;
import :activity_type;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::activitypub {

/** @brief One ActivityStreams activity with addressing and an opaque object. */
class Activity
{
public:
    Activity() = default;

    Activity(std::string id, ActivityType type, std::string actor, std::string object) :
        m_id{std::move(id)},
        m_type{type},
        m_actor{std::move(actor)},
        m_object{std::move(object)}
    {
    }

    [[nodiscard]] const std::string& id() const noexcept
    {
        return m_id;
    }

    [[nodiscard]] ActivityType type() const noexcept
    {
        return m_type;
    }

    [[nodiscard]] const std::string& actor() const noexcept
    {
        return m_actor;
    }

    [[nodiscard]] const std::string& object() const noexcept
    {
        return m_object;
    }

    Activity& add_to(std::string uri)
    {
        m_to.push_back(std::move(uri));
        return *this;
    }

    [[nodiscard]] const std::vector<std::string>& to() const noexcept
    {
        return m_to;
    }

    [[nodiscard]] bool valid() const noexcept
    {
        return !m_id.empty() && !m_actor.empty();
    }

private:
    std::string m_id{};
    ActivityType m_type{ActivityType::Create};
    std::string m_actor{};
    std::string m_object{};
    std::vector<std::string> m_to{};
};

} // namespace core::activitypub

#ifdef CONGELADO_TEST
namespace core::activitypub::tests {
using namespace boost::ut;

suite<"core_activitypub_activity"> core_activitypub_activity_suite = []
{
    "carries activity metadata"_test = []
    {
        Activity
            activity{"https://x/a/1", ActivityType::Create, "https://x/u/ada", "https://x/n/1"};
        activity.add_to("https://x/u/ada/followers");
        expect(activity.valid());
        expect(activity.type() == ActivityType::Create);
        expect(activity.to().size() == 1);
    };
};

} // namespace core::activitypub::tests
#endif
