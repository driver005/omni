export module blog_handlers:members_put_detail_model;
import std;
export namespace blog::handlers::members::put_detail::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
