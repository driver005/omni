export module blog_activitypub:builder;
import std;
import :activitypub;
export namespace blog::activitypub {
class ActivityPubServiceBuilder
{
public:
    [[nodiscard]] ActivityPubService build() const { return ActivityPubService{}; }
};
} // namespace blog::activitypub
