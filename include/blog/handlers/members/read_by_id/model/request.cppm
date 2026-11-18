export module blog_handlers:members_read_by_id_model;
import std;
export namespace blog::handlers::members::read_by_id::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
