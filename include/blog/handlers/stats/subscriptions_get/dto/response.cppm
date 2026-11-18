export module blog_handlers:stats_subscriptions_get_dto;
import std;
export namespace blog::handlers::stats::subscriptions_get::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
