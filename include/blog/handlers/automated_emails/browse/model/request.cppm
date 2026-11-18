export module blog_handlers:automated_emails_browse_model;
import std;
export namespace blog::handlers::automated_emails::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
