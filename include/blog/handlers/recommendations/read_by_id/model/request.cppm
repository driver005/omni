export module blog_handlers:recommendations_read_by_id_model;
import std;
export namespace blog::handlers::recommendations::read_by_id::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
