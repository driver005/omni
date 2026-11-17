export module core_activitypub:actor;

import std;
import :activity_type;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::activitypub {

/** @brief A remote or local ActivityPub actor with its endpoints. */
class Actor
{
public:
    Actor() = default;

    Actor(std::string id, std::string inbox, std::string outbox) :
        m_id{std::move(id)},
        m_inbox{std::move(inbox)},
        m_outbox{std::move(outbox)}
    {
    }

    [[nodiscard]] const std::string& id() const noexcept
    {
        return m_id;
    }

    [[nodiscard]] const std::string& inbox() const noexcept
    {
        return m_inbox;
    }

    [[nodiscard]] const std::string& outbox() const noexcept
    {
        return m_outbox;
    }

    Actor& followers(std::string url)
    {
        m_followers = std::move(url);
        return *this;
    }

    [[nodiscard]] const std::string& followers() const noexcept
    {
        return m_followers;
    }

    [[nodiscard]] bool valid() const noexcept
    {
        return !m_id.empty() && !m_inbox.empty();
    }

private:
    std::string m_id{};
    std::string m_inbox{};
    std::string m_outbox{};
    std::string m_followers{};
};

} // namespace core::activitypub

#ifdef CONGELADO_TEST
namespace core::activitypub::tests {
using namespace boost::ut;

suite<"core_activitypub_actor"> core_activitypub_actor_suite = []
{
    "carries actor endpoints"_test = []
    {
        Actor actor{"https://x/u/ada", "https://x/u/ada/inbox", "https://x/u/ada/outbox"};
        actor.followers("https://x/u/ada/followers");
        expect(actor.valid());
        expect(actor.inbox().ends_with("/inbox"));
        expect(actor.followers().ends_with("/followers"));
    };
};

} // namespace core::activitypub::tests
#endif
