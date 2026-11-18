export module blog_handlers:upgrades_read_by_id_model;
import std;
export namespace blog::handlers::upgrades::read_by_id::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
