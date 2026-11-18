export module blog_handlers:emails_recipient_failures_get_model;
import std;
export namespace blog::handlers::emails::recipient_failures_get::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
