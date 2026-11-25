module;
export module app_crypto:otp;

import std;
import :hash;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::crypto {

/** @brief Persistence surface for one-time codes.
 *
 *  app_keystore::RepositoryOneTimeStateStore implements this on top of the
 *  injected core::db::Database. The interface lives here so app_crypto stays
 *  independent of the database module; generators accept it by reference.
 */
class IOneTimeStateStore
{
public:
    virtual ~IOneTimeStateStore() = default;

    /** @brief Consumes a TOTP digest once; false means it was already used. */
    virtual bool consume_totp(std::string_view code_hash, std::int64_t now_seconds) = 0;

    /** @brief Records a verification/consumption event for audit. */
    virtual void record_consumption(
        std::string_view code_hash,
        std::string_view kind,
        std::int64_t at_seconds
    ) = 0;

    /** @brief Persists the hashed recovery codes issued to an owner. */
    virtual void
    save_recovery_codes(std::string_view owner, const std::vector<std::string>& code_hashes) = 0;

    /** @brief Consumes one recovery code; false means unknown or already used. */
    virtual bool consume_recovery_code(std::string_view owner, std::string_view code_hash) = 0;
};

/** @brief Domain-separated digests for stored one-time codes.
 *
 *  Only digests are persisted; plaintext codes are shown to the user once.
 */
class OneTimeCodeHasher
{
public:
    [[nodiscard]] std::string totp(std::string_view secret, std::string_view code) const
    {
        return KeyedHasher{std::string{secret}}.hash_with_domain(HashDomain::Otp, code);
    }

    [[nodiscard]] std::string recovery(std::string_view owner, std::string_view code) const
    {
        return KeyedHasher{std::string{owner}}.hash_with_domain(HashDomain::RecoveryCode, code);
    }
};

/** @brief Deterministic 6-digit time-step code stand-in.
 *
 *  A real TOTP uses HMAC-SHA1 over the time step; here the keyed FNV-1a digest
 *  of the decimal step is reduced modulo one million. When a state store is
 *  injected, verify() consumes a code exactly once and records the event.
 */
class TotpGenerator
{
public:
    TotpGenerator() = default;

    /** @brief Repository-backed generator; state writes go through the store. */
    explicit TotpGenerator(IOneTimeStateStore& store) :
        m_store{&store}
    {
    }

    [[nodiscard]] std::string generate(std::string_view secret, std::uint64_t step)
    {
        const KeyedHasher hasher{std::string{secret}};
        const std::string digest = hasher.hash(std::to_string(step));
        const std::string tail = digest.substr(digest.size() - 8);
        const std::uint64_t value = parse_hex(tail);
        return format_code(value % 1'000'000ULL);
    }

    /** @brief Current 30-second time step for a unix timestamp. */
    [[nodiscard]] std::uint64_t
    current_step(std::int64_t unix_seconds, std::uint64_t period_seconds = 30) const
    {
        if (period_seconds == 0) {
            return 0;
        }
        const std::uint64_t stamp =
            unix_seconds < 0 ? 0ULL : static_cast<std::uint64_t>(unix_seconds);
        return stamp / period_seconds;
    }

    /** @brief Verifies a code within a step window, consuming it once if backed. */
    [[nodiscard]] bool verify(
        std::string_view secret,
        std::uint64_t step,
        std::string_view code,
        std::int64_t now_seconds = 0,
        int window = 1
    )
    {
        if (code.size() != 6) {
            return false;
        }
        const int radius = window < 0 ? 0 : window;
        for (int delta{-radius}; delta <= radius; ++delta) {
            const std::uint64_t candidate_step = delta < 0
                                                     ? step - static_cast<std::uint64_t>(-delta)
                                                     : step + static_cast<std::uint64_t>(delta);
            if (generate(secret, candidate_step) != code) {
                continue;
            }
            if (m_store == nullptr) {
                return true;
            }
            const std::string hash = m_hasher.totp(secret, code);
            if (!m_store->consume_totp(hash, now_seconds)) {
                return false;
            }
            m_store->record_consumption(hash, "totp", now_seconds);
            return true;
        }
        return false;
    }

private:
    [[nodiscard]] std::uint64_t parse_hex(std::string_view text) const
    {
        std::uint64_t value{0};
        for (const char c: text) {
            value = (value << 4) | static_cast<std::uint64_t>(nibble(c));
        }
        return value;
    }

    [[nodiscard]] unsigned nibble(char c) const
    {
        if (c >= '0' && c <= '9') {
            return static_cast<unsigned>(c - '0');
        }
        return static_cast<unsigned>(c - 'a') + 10U;
    }

    [[nodiscard]] std::string format_code(std::uint64_t value) const
    {
        std::string out{"000000"};
        for (int i{5}; i >= 0; --i) {
            out[static_cast<std::size_t>(i)] = static_cast<char>('0' + (value % 10ULL));
            value /= 10ULL;
        }
        return out;
    }

    IOneTimeStateStore* m_store{nullptr};
    OneTimeCodeHasher m_hasher{};
};

/** @brief Deterministic recovery codes shaped like XXXX-XXXX.
 *
 *  generate() keeps the legacy no-persistence behavior. generate_for() and
 *  verify() require an injected state store so codes are hashed into the
 *  repository and each one is consumed at most once.
 */
class RecoveryCodeGenerator
{
public:
    explicit RecoveryCodeGenerator(std::uint64_t seed = 0) :
        m_seed{seed}
    {
    }

    /** @brief Repository-backed generator for a named owner. */
    RecoveryCodeGenerator(std::uint64_t seed, std::string owner, IOneTimeStateStore& store) :
        m_seed{seed},
        m_owner{std::move(owner)},
        m_store{&store}
    {
    }

    [[nodiscard]] std::vector<std::string> generate(std::size_t count) const
    {
        std::vector<std::string> codes{};
        codes.reserve(count);
        for (std::size_t i{0}; i < count; ++i) {
            codes.push_back(one(i));
        }
        return codes;
    }

    /** @brief Generates plaintext codes for the user and stores only their digests. */
    [[nodiscard]] std::vector<std::string> generate_for(std::size_t count) const
    {
        const std::vector<std::string> codes = generate(count);
        if (m_store == nullptr) {
            return codes;
        }
        std::vector<std::string> hashes{};
        hashes.reserve(codes.size());
        for (const std::string& code: codes) {
            hashes.push_back(m_hasher.recovery(m_owner, code));
        }
        m_store->save_recovery_codes(m_owner, hashes);
        return codes;
    }

    /** @brief Consumes a recovery code once; requires a state store. */
    [[nodiscard]] bool verify(std::string_view code, std::int64_t now_seconds = 0) const
    {
        if (m_store == nullptr || code.size() != 9 || code[4] != '-') {
            return false;
        }
        const std::string hash = m_hasher.recovery(m_owner, code);
        if (!m_store->consume_recovery_code(m_owner, hash)) {
            return false;
        }
        m_store->record_consumption(hash, "recovery_code", now_seconds);
        return true;
    }

private:
    [[nodiscard]] std::string one(std::size_t index) const
    {
        constexpr std::string_view alphabet{"ABCDEFGHJKLMNPQRSTUVWXYZ23456789"};
        const std::uint64_t mixed = static_cast<std::uint64_t>(std::hash<std::uint64_t>{}(
            m_seed ^ (static_cast<std::uint64_t>(index) * 0x9E'37'79'B9'7F'4A'7C'15ULL)
        ));

        std::uint64_t value{mixed};
        std::string out{};
        out.reserve(9);
        for (int group{0}; group < 2; ++group) {
            if (group == 1) {
                out.push_back('-');
            }
            for (int pos{0}; pos < 4; ++pos) {
                out.push_back(alphabet[static_cast<std::size_t>(value & 0x1FULL)]);
                value >>= 5;
            }
        }
        return out;
    }

    std::uint64_t m_seed{0};
    std::string m_owner{};
    IOneTimeStateStore* m_store{nullptr};
    OneTimeCodeHasher m_hasher{};
};

} // namespace app::crypto

#ifdef CONGELADO_TEST
namespace app::crypto::tests {
using namespace boost::ut;

namespace {
    class TestStateStore final : public IOneTimeStateStore
    {
    public:
        bool consume_totp(std::string_view code_hash, std::int64_t) override
        {
            if (m_consumed.contains(std::string{code_hash})) {
                return false;
            }
            m_consumed.insert(std::string{code_hash});
            return true;
        }

        void record_consumption(std::string_view, std::string_view, std::int64_t) override
        {
            ++m_events;
        }

        void save_recovery_codes(
            std::string_view,
            const std::vector<std::string>& code_hashes
        ) override
        {
            m_codes.insert(m_codes.end(), code_hashes.begin(), code_hashes.end());
        }

        bool consume_recovery_code(std::string_view, std::string_view code_hash) override
        {
            const auto it = std::find(m_codes.begin(), m_codes.end(), std::string{code_hash});
            if (it == m_codes.end()) {
                return false;
            }
            m_codes.erase(it);
            return true;
        }

        [[nodiscard]] std::size_t events() const
        {
            return m_events;
        }

    private:
        std::set<std::string> m_consumed{};
        std::vector<std::string> m_codes{};
        std::size_t m_events{0};
    };
} // namespace

suite<"app_crypto_otp"> otp_suite = []
{
    "totp code is six decimal digits and deterministic"_test = []
    {
        TotpGenerator gen;
        const std::string code = gen.generate("totp-secret", 123'456'789ULL);
        expect(code.size() == 6);
        for (const char c: code) {
            expect(c >= '0' && c <= '9');
        }
        expect(code == gen.generate("totp-secret", 123'456'789ULL));
        expect(code != gen.generate("other-secret", 123'456'789ULL));
    };

    "recovery codes have stable count, shape and sequence"_test = []
    {
        const RecoveryCodeGenerator gen{42};
        const std::vector<std::string> codes = gen.generate(3);
        expect(codes.size() == 3);
        for (const std::string& code: codes) {
            expect(code.size() == 9);
            expect(code[4] == '-');
            expect(code[0] != '-');
        }
        expect(codes == RecoveryCodeGenerator{42}.generate(3));
        expect(codes != RecoveryCodeGenerator{43}.generate(3));
        expect(RecoveryCodeGenerator{}.generate(0).empty());
    };

    "totp verification consumes a code once through the store"_test = []
    {
        TestStateStore store;
        TotpGenerator gen{store};
        const std::uint64_t step = 123'456'789ULL;
        const std::string code = gen.generate("totp-secret", step);
        expect(gen.verify("totp-secret", step, code, 1'234'567'890));
        expect(!gen.verify("totp-secret", step, code, 1'234'567'890));
        expect(store.events() == 1);
        expect(!gen.verify("totp-secret", step, "000000", 1'234'567'890));
    };

    "recovery codes are stored hashed and consumed once"_test = []
    {
        TestStateStore store;
        RecoveryCodeGenerator gen{7, "merchant_1", store};
        const std::vector<std::string> codes = gen.generate_for(2);
        expect(codes.size() == 2);
        expect(gen.verify(codes.front()));
        expect(!gen.verify(codes.front()));
        expect(gen.verify(codes.back()));
        expect(!gen.verify("AAAA-AAAA"));
    };
};

} // namespace app::crypto::tests
#endif
