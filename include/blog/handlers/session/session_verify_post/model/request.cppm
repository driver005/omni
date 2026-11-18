export module blog_handlers:session_session_verify_post_model;
import std;
export namespace blog::handlers::session::session_verify_post::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
