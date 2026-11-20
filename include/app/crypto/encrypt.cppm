module;
export module app_crypto:encrypt;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::crypto {

/** @brief XOR-stream stand-in for the upstream AES key-wrap step.
 *
 *  NOT AES. A deterministic key schedule is expanded from the key bytes plus a
 *  hashed xorshift tail, then combined with a position-dependent rotation to
 *  form a repeating mask. Encrypt and decrypt are the same operation, so the
 *  shape and round-trip behavior match the upstream API, but there is no
 *  block cipher, no padding and no authentication. Do not use for real data.
 */
class AesWrap
{
public:
    AesWrap() = default;

    [[nodiscard]] std::string encrypt(std::string_view plaintext, std::string_view key)
    {
        return transform(plaintext, key);
    }

    [[nodiscard]] std::string decrypt(std::string_view ciphertext, std::string_view key)
    {
        return transform(ciphertext, key);
    }

private:
    [[nodiscard]] std::string transform(std::string_view input, std::string_view key) const
    {
        const std::vector<unsigned char> schedule = key_schedule(key);
        std::string out{};
        out.reserve(input.size());
        for (std::size_t i{0}; i < input.size(); ++i) {
            const unsigned char mask = static_cast<unsigned char>(
                schedule[i % schedule.size()] ^
                rotate_byte(static_cast<unsigned char>(i), key));
            out.push_back(static_cast<char>(static_cast<unsigned char>(input[i]) ^ mask));
        }
        return out;
    }

    [[nodiscard]] std::vector<unsigned char> key_schedule(std::string_view key) const
    {
        std::vector<unsigned char> words{};
        words.reserve(key.size() + 32);
        for (const char c : key) {
            words.push_back(static_cast<unsigned char>(c));
        }
        std::uint64_t state = static_cast<std::uint64_t>(std::hash<std::string_view>{}(key)) ^
                              0x9E3779B97F4A7C15ULL;
        for (std::size_t i{0}; i < 32; ++i) {
            state ^= state << 13;
            state ^= state >> 7;
            state ^= state << 17;
            words.push_back(static_cast<unsigned char>(state & 0xFFU));
        }
        if (words.empty()) {
            words.push_back(0x5AU);
        }
        return words;
    }

    [[nodiscard]] unsigned char rotate_byte(unsigned char value, std::string_view key) const
    {
        const unsigned amount = static_cast<unsigned>(key.size() & 7U);
        return static_cast<unsigned char>((value << amount) | (value >> (8U - amount)));
    }
};

} // namespace app::crypto

#ifdef CONGELADO_TEST
namespace app::crypto::tests {
using namespace boost::ut;

suite<"app_crypto_encrypt"> encrypt_suite = []
{
    "encrypt/decrypt round-trips and is deterministic"_test = []
    {
        AesWrap wrap;
        const std::string plaintext{"sensitive-card-data"};
        const std::string ciphertext = wrap.encrypt(plaintext, "wrap-key");
        expect(wrap.decrypt(ciphertext, "wrap-key") == plaintext);
        expect(ciphertext == wrap.encrypt(plaintext, "wrap-key"));
        expect(ciphertext.size() == plaintext.size());
    };

    "ciphertext depends on the key and empty input stays empty"_test = []
    {
        AesWrap wrap;
        expect(wrap.encrypt("payload", "key-a") != wrap.encrypt("payload", "key-b"));
        expect(wrap.encrypt("", "key-a").empty());
        expect(wrap.decrypt("", "key-a").empty());
    };
};

} // namespace app::crypto::tests
#endif
