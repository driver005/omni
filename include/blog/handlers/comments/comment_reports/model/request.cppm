export module blog_handlers:comments_comment_reports_model;
import std;
export namespace blog::handlers::comments::comment_reports::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
