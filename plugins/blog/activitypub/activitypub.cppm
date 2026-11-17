export module blog_activitypub:activitypub;

import std;
import core_activitypub;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::activitypub {

/** @brief Ghost ActivityPub bridge over the core ActivityPub facade. */
class ActivityPubService
{
public:
    void publish_note(std::string id, std::string actor, std::string content)
    {
        core::activitypub::Activity activity{
            std::move(id), core::activitypub::ActivityType::Create,
            std::move(actor), std::move(content)
        };
        m_ap.publish(std::move(activity));
    }

    [[nodiscard]] std::size_t published() const noexcept
    {
        return m_ap.outbox_size();
    }

    [[nodiscard]] bool verify(std::string_view message, std::span<const unsigned char> signature) const
    {
        return m_ap.verify(message, signature);
    }

private:
    core::activitypub::ActivityPub m_ap{};
};

} // namespace blog::activitypub
#ifdef CONGELADO_TEST
namespace blog::activitypub::tests {
using namespace boost::ut;
suite<"blog_activitypub_activitypub"> blog_activitypub_activitypub_suite = []
{
    "publishes a note"_test = []
    {
        ActivityPubService service;
        service.publish_note("1", "https://x/u/ada", "hello");
        expect(service.published() == 1);
    };
};
} // namespace blog::activitypub::tests
#endif
