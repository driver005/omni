import std;
import core_outbox;

using namespace core::outbox;

namespace {

class Check
{
public:
    void equal(std::string_view actual, std::string_view expected, std::string_view label)
    {
        if (actual != expected) {
            std::println(std::cerr, "FAIL {}: expected '{}', got '{}'", label, expected, actual);
            ++m_failures;
        }
    }

    void yes(bool condition, std::string_view label)
    {
        if (!condition) {
            std::println(std::cerr, "FAIL {}", label);
            ++m_failures;
        }
    }

    int failures() const
    {
        return m_failures;
    }

private:
    int m_failures{0};
};

class CountingAdapter final : public Adapter
{
public:
    void dispatch(const Event& event) override
    {
        ++count;
        last_type = event.type;
        types.push_back(event.type);
    }

    int count{0};
    std::string last_type;
    std::vector<std::string> types;
};

} // namespace

int main()
{
    Check check;

    // Registered once for the whole process (global adapter registry). Assertions below
    // use deltas so earlier cases don't pollute later ones.
    auto adapter = std::make_shared<CountingAdapter>();
    register_adapter(adapter);
    register_adapter(nullptr); // null adapters are ignored

    // 1. Basic emit -> pending -> drain, with adapter dispatch.
    {
        Outbox box;
        check.yes(box.pending() == 0, "starts empty");
        box.emit(Event{"post.published", R"({"id":1})", "post-1"});
        check.yes(box.pending() == 1, "pending after emit");

        const int before = adapter->count;
        auto drained = box.drain();
        check.yes(drained.size() == 1, "drain returns one event");
        check.equal(drained[0].type, "post.published", "drained type");
        check.equal(drained[0].payload, R"({"id":1})", "drained payload");
        check.equal(drained[0].aggregate_id, "post-1", "drained aggregate id");
        check.yes(box.pending() == 0, "empty after drain");
        check.yes(adapter->count - before == 1, "adapter dispatched once");
        check.equal(adapter->last_type, "post.published", "adapter saw event type");
    }

    // 2. Transaction: staged events are invisible until commit.
    {
        Outbox box;
        box.begin_transaction();
        box.emit(Event{"a", "{}", "1"});
        box.emit(Event{"b", "{}", "2"});

        const int before = adapter->count;
        check.yes(box.pending() == 0, "staged events not pending");
        check.yes(box.drain().empty(), "staged events not drained");
        check.yes(adapter->count == before, "staged events not dispatched");

        box.commit();
        check.yes(box.pending() == 2, "pending after commit");

        auto drained = box.drain();
        check.yes(drained.size() == 2, "drain after commit returns both");
        check.yes(drained[0].type == "a" && drained[1].type == "b", "emit order preserved");
        check.yes(adapter->count - before == 2, "dispatched only after commit");
    }

    // 3. Rollback discards staged events.
    {
        Outbox box;
        box.begin_transaction();
        box.emit(Event{"x", "{}", "9"});

        const int before = adapter->count;
        box.rollback();
        check.yes(box.pending() == 0, "nothing pending after rollback");
        check.yes(box.drain().empty(), "nothing drained after rollback");
        check.yes(adapter->count == before, "no dispatch after rollback");
    }

    // 4. Nested transactions: inner commit merges into parent; both land on outer commit.
    {
        Outbox box;
        box.begin_transaction();
        box.emit(Event{"outer", "{}", "o"});
        box.begin_transaction();
        box.emit(Event{"inner", "{}", "i"});
        box.commit(); // inner -> parent buffer
        check.yes(box.pending() == 0, "still invisible before outer commit");
        box.commit(); // parent -> pending

        auto drained = box.drain();
        check.yes(drained.size() == 2, "nested commit yields both events");
        check.yes(drained[0].type == "outer" && drained[1].type == "inner", "nested order preserved");
    }

    // 5. Inner rollback discards only its own staged events.
    {
        Outbox box;
        box.begin_transaction();
        box.emit(Event{"keep", "{}", "k"});
        box.begin_transaction();
        box.emit(Event{"drop", "{}", "d"});
        box.rollback();
        box.commit();
        auto drained = box.drain();
        check.yes(drained.size() == 1, "inner rollback keeps only parent event");
        check.equal(drained[0].type, "keep", "surviving event is parent's");
    }

    // 6. Context-local: two Outbox instances are independent.
    {
        Outbox a;
        Outbox b;
        a.emit(Event{"a1", "{}", "1"});
        check.yes(a.pending() == 1, "first context has its event");
        check.yes(b.pending() == 0, "second context unaffected");
        check.yes(b.drain().empty(), "second context drains empty");
        a.begin_transaction();
        a.emit(Event{"a2", "{}", "2"});
        check.yes(a.pending() == 1, "transaction staged in first context only");
        check.yes(b.pending() == 0, "second context still independent");
    }

    // 7. Empty commit/rollback are safe no-ops.
    {
        Outbox box;
        box.commit();
        box.rollback();
        check.yes(box.pending() == 0, "no-op transactions are safe");
    }

    check.yes(adapter->count == 6, "total adapter dispatches across all cases");

    return check.failures() == 0 ? 0 : 1;
}
