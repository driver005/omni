module;
export module app_locking:api_lock;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::locking {

/** @brief Value snapshot of one acquired API lock. */
class ApiLock
{
public:
    ApiLock() = default;

    void set_key(std::string value) { m_key = std::move(value); }
    [[nodiscard]] const std::string& get_key() const noexcept { return m_key; }
    void set_owner(std::string value) { m_owner = std::move(value); }
    [[nodiscard]] const std::string& get_owner() const noexcept { return m_owner; }
    void set_acquired_at(std::int64_t value) { m_acquired_at = std::move(value); }
    [[nodiscard]] const std::int64_t& get_acquired_at() const noexcept { return m_acquired_at; }
    void set_expires_at(std::int64_t value) { m_expires_at = std::move(value); }
    [[nodiscard]] const std::int64_t& get_expires_at() const noexcept { return m_expires_at; }

    std::string m_key{};
    std::string m_owner{};
    std::int64_t m_acquired_at{};
    std::int64_t m_expires_at{};
};

} // namespace app::locking

#ifdef CONGELADO_TEST
namespace app::locking::tests {

using namespace boost::ut;

suite<"app_locking_api_lock"> api_lock_suite = []
{
    "carries key, owner and expiry window"_test = []
    {
        ApiLock lock{};
        lock.set_key("payment:1");
        lock.set_owner("handler");
        lock.set_acquired_at(100);
        lock.set_expires_at(130);

        expect(lock.get_key() == "payment:1");
        expect(lock.get_owner() == "handler");
        expect(lock.get_acquired_at() == 100);
        expect(lock.get_expires_at() == 130);
    };
};

} // namespace app::locking::tests
#endif
