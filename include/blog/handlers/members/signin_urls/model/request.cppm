export module blog_handlers:members_signin_urls_model;
import std;
export namespace blog::handlers::members::signin_urls::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
