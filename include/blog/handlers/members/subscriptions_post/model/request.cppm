export module blog_handlers:members_subscriptions_post_model;
import std;
export namespace blog::handlers::members::subscriptions_post::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
