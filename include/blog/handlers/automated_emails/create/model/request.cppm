export module blog_handlers:automated_emails_create_model;
import std;
export namespace blog::handlers::automated_emails::create::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
