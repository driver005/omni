export module core_bridge:bridge_status;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::bridge {

/** @brief Outcome codes for bridge operations. */
enum class BridgeStatus : std::uint8_t
{
    OK,
    NOT_FOUND,
    TYPE_MISMATCH,
    LOAD_FAILED,
    SYMBOL_MISSING,
    ALREADY_LOADED
};

/** @brief Instance functor naming a BridgeStatus — no free functions. */
export class BridgeStatusName
{
public:
    [[nodiscard]] constexpr std::string_view operator()(BridgeStatus status) const noexcept
    {
        switch (status) {
            case BridgeStatus::OK:
                return "ok";
            case BridgeStatus::NOT_FOUND:
                return "not_found";
            case BridgeStatus::TYPE_MISMATCH:
                return "type_mismatch";
            case BridgeStatus::LOAD_FAILED:
                return "load_failed";
            case BridgeStatus::SYMBOL_MISSING:
                return "symbol_missing";
            case BridgeStatus::ALREADY_LOADED:
                return "already_loaded";
        }
        return "unknown";
    }
};

} // namespace core::bridge

#ifdef CONGELADO_TEST
namespace core::bridge::tests {
using namespace boost::ut;

suite<"core_bridge_status"> core_bridge_status_suite = []
{
    "names every status"_test = []
    {
        const BridgeStatusName name;
        expect(name(BridgeStatus::OK) == "ok");
        expect(name(BridgeStatus::NOT_FOUND) == "not_found");
        expect(name(BridgeStatus::TYPE_MISMATCH) == "type_mismatch");
        expect(name(BridgeStatus::LOAD_FAILED) == "load_failed");
        expect(name(BridgeStatus::SYMBOL_MISSING) == "symbol_missing");
        expect(name(BridgeStatus::ALREADY_LOADED) == "already_loaded");
    };
};

} // namespace core::bridge::tests
#endif
