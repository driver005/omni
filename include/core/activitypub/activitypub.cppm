export module core_activitypub:activitypub;

import std;
import :activity;
import :actor;
import :runtime;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::activitypub {

/** @brief Opaque ActivityPub handle. All members inline, shared_ptr pImpl. */
class ActivityPub
{
public:
    ActivityPub() :
        m_runtime{std::make_shared<ActivityPubRuntime>()}
    {
    }

    ActivityPub(const ActivityPub&) = delete;
    ActivityPub& operator=(const ActivityPub&) = delete;
    ActivityPub(ActivityPub&&) noexcept = default;
    ActivityPub& operator=(ActivityPub&&) noexcept = default;
    ~ActivityPub() = default;

    void receive(Activity activity)
    {
        m_runtime->inbox().receive(std::move(activity));
    }

    void publish(Activity activity)
    {
        m_runtime->outbox().publish(std::move(activity));
    }

    [[nodiscard]] std::size_t inbox_size() const noexcept
    {
        return m_runtime->inbox().size();
    }

    [[nodiscard]] std::size_t outbox_size() const noexcept
    {
        return m_runtime->outbox().size();
    }

    [[nodiscard]] std::vector<unsigned char> sign(std::string_view message) const
    {
        return m_runtime->signature().sign(message);
    }

    [[nodiscard]] bool verify(
        std::string_view message,
        std::span<const unsigned char> signature
    ) const
    {
        return m_runtime->signature().verify(message, signature);
    }

private:
    std::shared_ptr<ActivityPubRuntime> m_runtime{};
};

} // namespace core::activitypub

#ifdef CONGELADO_TEST
namespace core::activitypub::tests {
using namespace boost::ut;

suite<"core_activitypub_activitypub"> core_activitypub_activitypub_suite = []
{
    "facade receives, publishes and signs"_test = []
    {
        ActivityPub ap;
        ap.receive(Activity{"1", ActivityType::Create, "https://x/u/bob", "o1"});
        ap.publish(Activity{"2", ActivityType::Like, "https://x/u/ada", "o1"});
        expect(ap.inbox_size() == 1);
        expect(ap.outbox_size() == 1);
        const auto signature = ap.sign("message");
        expect(ap.verify("message", signature));
    };
};

} // namespace core::activitypub::tests
#endif
