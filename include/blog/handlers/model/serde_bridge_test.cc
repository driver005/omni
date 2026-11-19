import std;
import blog_handlers;
import core_database;
import memory_backends;

int main()
{
    core::db::Database db = core::db::DatabaseBuilder{}.build();
    db.use_backend(std::make_unique<core::db::InMemoryBackend>());
    db.register_table("recommendations");

    blog::model::Recommendation rec;
    rec.set_url("https://ghost.org");
    rec.set_title("Ghost");
    db.insert(rec);

    const auto all = db.all<blog::model::Recommendation>();
    if (all.size() != 1) return 1;
    if (all.front().get_title() != "Ghost") return 2;
    return 0;
}
