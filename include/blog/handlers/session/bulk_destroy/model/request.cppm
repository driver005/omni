export module blog_handlers:session_bulk_destroy_model;
import std;
export namespace blog::handlers::session::bulk_destroy::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
