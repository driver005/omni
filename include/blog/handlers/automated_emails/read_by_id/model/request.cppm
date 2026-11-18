export module blog_handlers:automated_emails_read_by_id_model;
import std;
export namespace blog::handlers::automated_emails::read_by_id::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
