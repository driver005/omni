export module blog_handlers:upgrades_create_model;
import std;
export namespace blog::handlers::upgrades::create::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
