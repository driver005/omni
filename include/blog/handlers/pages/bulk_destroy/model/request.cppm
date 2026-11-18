export module blog_handlers:pages_bulk_destroy_model;
import std;
export namespace blog::handlers::pages::bulk_destroy::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
