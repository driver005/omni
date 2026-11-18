export module blog_handlers:automated_emails_edit_model;
import std;
export namespace blog::handlers::automated_emails::edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
