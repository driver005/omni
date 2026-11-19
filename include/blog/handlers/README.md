# handlers layout

Every Ghost domain lives under `include/blog/handlers/<domain>/`.

```text
handlers/<domain>/
├── routes.cppm                  # route installation for this domain (module blog_handlers_routes:<domain>_routes)
├── <operation>/
│   ├── handler.cppm             # hand-written handler, no CoreHandler dependency
│   ├── model/                   # request/input models
│   ├── dto/                     # response DTOs and error seeds
│   └── workflow/
│       ├── step/                # core::workflow::WorkflowStep implementations
│       └── flow/                # WorkflowBuilder definitions
```

## Rules

- No central `api/` directory. Route files are part of their domain folder.
- One handler folder per concrete route operation, not one god handler.
- Handlers receive injected services: `BlogContext`, `core::cache::Cache`, `StripeHandler`, or nothing.
- No `SpecialEndpoints`, `SpecialResult`, `EndpointResponse`, `ModelJson` or `GhostJson`. There is no central special-endpoint dispatch.
- Stateless endpoint bodies are built inline in each route step with `core_serde` DTOs. Ghost fixtures live in `blog_context:ghost_fixtures`, not in `blog_dto`.
- `blog_handlers_middleware:serde` validates `content-type` / `accept` on each domain router and drives `serde::Ser::serialize/deserialize` with the media type taken from the request.
- Shared code is limited to `middleware/`, `mutation.cppm`, `stripe.cppm`, the three domain `operations.cppm` files, and `all.cppm`.
- Each `operations.cppm` exports one class (`MembersOperations`, `EmailsOperations`, `AutomationsOperations`) holding a `core::cache::Cache&`. Instances live in `BlogHandlers`; stateful steps hold a pointer to the owning operations object.
- Cross-request mutable data lives in the `core::cache::Cache` owned by `BlogHandlers`, under namespaced keys (`members:*`, `emails:*`, `automations:*`), never in domain object fields.

Route lambdas bind to one exact handler:

```cpp
.get([h](req,res,send){ h->browse().handle(req,res,send); })
```
