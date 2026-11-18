export module blog_handlers:incoming_recommendations_browse_model;
import std;
export namespace blog::handlers::incoming_recommendations::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
