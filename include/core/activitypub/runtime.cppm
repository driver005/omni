export module core_activitypub:runtime;

import std;
import :inbox;
import :outbox;
import :http_signature;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::activitypub {
class ActivityPubRuntime;
}

namespace core::activitypub {

/** @brief Internal state owned by the facade. */
class ActivityPubRuntime
{
public:
    Inbox& inbox() noexcept
    {
        return m_inbox;
    }

    Outbox& outbox() noexcept
    {
        return m_outbox;
    }

    HttpSignature& signature() noexcept
    {
        return m_signature;
    }

    [[nodiscard]] const Inbox& inbox() const noexcept
    {
        return m_inbox;
    }

    [[nodiscard]] const Outbox& outbox() const noexcept
    {
        return m_outbox;
    }

private:
    Inbox m_inbox{};
    Outbox m_outbox{};
    HttpSignature m_signature{};
};

} // namespace core::activitypub

#ifdef CONGELADO_TEST
namespace core::activitypub::tests {
using namespace boost::ut;

suite<"core_activitypub_runtime"> core_activitypub_runtime_suite = []
{
    "owns inbox, outbox and signature"_test = []
    {
        ActivityPubRuntime runtime;
        expect(runtime.inbox().size() == 0);
        expect(runtime.outbox().size() == 0);
        expect(runtime.signature().valid());
    };
};

} // namespace core::activitypub::tests
#endif
