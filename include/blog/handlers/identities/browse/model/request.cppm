export module blog_handlers:identities_browse_model;
import std;
export namespace blog::handlers::identities::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
