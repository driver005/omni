module;
export module app_crypto:ids;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::crypto {

/** @brief Instance-scoped, deterministic identifier factory.
 *
 *  Output is shaped like the identifiers used by the upstream application, but
 *  it is derived from an in-memory counter mixed with std::hash. Identifiers
 *  are reproducible within a process and carry no security properties.
 */
class IdGenerator
{
public:
    IdGenerator() = default;

    /** @brief Fixed-length API key identifier: "ak_" + 32 hex digits. */
    [[nodiscard]] std::string api_key_id()
    {
        std::string out{"ak_"};
        out += hex_word(next());
        out += hex_word(next());
        return out;
    }

    /** @brief Fixed-length key identifier: "key_" + 16 hex digits. */
    [[nodiscard]] std::string key_id()
    {
        std::string out{"key_"};
        out += hex_word(next());
        return out;
    }

    /** @brief Fixed-length payment identifier: prefix + 20 hex digits. */
    [[nodiscard]] std::string payment_id(std::string_view prefix = "pay_")
    {
        std::string out{prefix};
        out += hex_word(next());
        const std::string tail = hex_word(next());
        out.append(tail, 0, 4);
        return out;
    }

    /** @brief Canonical 36-char UUID-shaped string with version 4 and a valid variant nibble. */
    [[nodiscard]] std::string uuid_v4()
    {
        std::string raw = hex_word(next());
        raw += hex_word(next());
        raw[12] = '4';
        constexpr std::string_view variants{"89ab"};
        raw[16] = variants[static_cast<std::size_t>(next() & 0x3ULL)];

        std::string out{};
        out.reserve(36);
        out.append(raw, 0, 8);
        out.push_back('-');
        out.append(raw, 8, 4);
        out.push_back('-');
        out.append(raw, 12, 4);
        out.push_back('-');
        out.append(raw, 16, 4);
        out.push_back('-');
        out.append(raw, 20, 12);
        return out;
    }

private:
    [[nodiscard]] std::uint64_t next()
    {
        ++m_counter;
        const std::uint64_t mixed = static_cast<std::uint64_t>(
            std::hash<std::uint64_t>{}(m_counter ^ 0x9E3779B97F4A7C15ULL));
        return mixed ^ (m_counter << 32);
    }

    [[nodiscard]] std::string hex_word(std::uint64_t value) const
    {
        constexpr std::string_view digits{"0123456789abcdef"};
        std::string out{};
        out.reserve(16);
        for (int shift{60}; shift >= 0; shift -= 4) {
            out.push_back(digits[static_cast<std::size_t>((value >> shift) & 0xFULL)]);
        }
        return out;
    }

    std::uint64_t m_counter{0};
};

} // namespace app::crypto

#ifdef CONGELADO_TEST
namespace app::crypto::tests {
using namespace boost::ut;

suite<"app_crypto_ids"> ids_suite = []
{
    "identifiers have the expected fixed lengths and prefixes"_test = []
    {
        IdGenerator gen;
        expect(gen.api_key_id().size() == 35);
        expect(gen.api_key_id().starts_with("ak_"));
        expect(gen.key_id().size() == 20);
        expect(gen.key_id().starts_with("key_"));
        expect(gen.payment_id().size() == 24);
        expect(gen.payment_id().starts_with("pay_"));
        expect(gen.payment_id("txn_").starts_with("txn_"));
        expect(gen.uuid_v4().size() == 36);
    };

    "uuid_v4 has canonical shape, version 4 and a valid variant"_test = []
    {
        const std::string id = IdGenerator{}.uuid_v4();
        expect(id[8] == '-');
        expect(id[13] == '-');
        expect(id[18] == '-');
        expect(id[23] == '-');
        expect(id[14] == '4');
        expect(id[19] == '8' || id[19] == '9' || id[19] == 'a' || id[19] == 'b');
    };

    "two generators produce the same sequence"_test = []
    {
        IdGenerator lhs;
        IdGenerator rhs;
        expect(lhs.api_key_id() == rhs.api_key_id());
        expect(lhs.key_id() == rhs.key_id());
        expect(lhs.payment_id() == rhs.payment_id());
        expect(lhs.uuid_v4() == rhs.uuid_v4());
    };
};

} // namespace app::crypto::tests
#endif
