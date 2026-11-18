export module blog_handlers_posts_browse:dto_errors;

import std;
import core_serde;
import blog_handlers_middleware;

export namespace blog::handlers::posts::browse::dto {

/** @brief Per-handler error seeds, built from the shared middleware error types. */
using ErrorItem = blog::handlers::middleware::ErrorItem;
using ErrorEnvelope = blog::handlers::middleware::ErrorEnvelope;

[[nodiscard]] inline ErrorEnvelope not_found()
{
    ErrorEnvelope envelope;
    envelope.errors.push_back(ErrorItem{
        .message = "Not Found",
        .type = "NotFoundError"
    });
    return envelope;
}

} // namespace blog::handlers::posts::browse::dto
