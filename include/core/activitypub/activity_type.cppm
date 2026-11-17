export module core_activitypub:activity_type;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::activitypub {

/** @brief ActivityStreams 2.0 activity/object types used by the abstraction. */
enum class ActivityType : std::uint8_t
{
    Create,
    Update,
    Delete,
    Follow,
    Accept,
    Reject,
    Like,
    Announce,
    Undo,
    Note,
    Article,
    Person
};

/** @brief Instance functor naming an ActivityType for JSON serialization. */
class ActivityTypeName
{
public:
    [[nodiscard]] std::string_view operator()(ActivityType type) const noexcept
    {
        switch (type) {
            case ActivityType::Create:
                return "Create";
            case ActivityType::Update:
                return "Update";
            case ActivityType::Delete:
                return "Delete";
            case ActivityType::Follow:
                return "Follow";
            case ActivityType::Accept:
                return "Accept";
            case ActivityType::Reject:
                return "Reject";
            case ActivityType::Like:
                return "Like";
            case ActivityType::Announce:
                return "Announce";
            case ActivityType::Undo:
                return "Undo";
            case ActivityType::Note:
                return "Note";
            case ActivityType::Article:
                return "Article";
            case ActivityType::Person:
                return "Person";
        }
        return "Unknown";
    }
};

} // namespace core::activitypub

#ifdef CONGELADO_TEST
namespace core::activitypub::tests {
using namespace boost::ut;

suite<"core_activitypub_activity_type"> core_activitypub_activity_type_suite = []
{
    "names activity types"_test = []
    {
        const ActivityTypeName name;
        expect(name(ActivityType::Create) == "Create");
        expect(name(ActivityType::Accept) == "Accept");
        expect(name(ActivityType::Person) == "Person");
    };
};

} // namespace core::activitypub::tests
#endif
