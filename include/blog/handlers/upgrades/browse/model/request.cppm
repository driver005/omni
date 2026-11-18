export module blog_handlers:upgrades_browse_model;
import std;
export namespace blog::handlers::upgrades::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
