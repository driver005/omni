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
- Handlers receive injected services (`BlogContext`, `SpecialEndpoints`, `MutationStore`).
- `EndpointResponse` is a plain status + body value returned by domain service methods; each route folder calls the method it needs directly. There is no central special-endpoint dispatch and no `SpecialResult` type.
- Shared code is limited to `middleware/`, `mutation.cppm`, `special.cppm`, `stripe.cppm`, `all.cppm`.

Route lambdas bind to one exact handler:

```cpp
.get([h](req,res,send){ h->browse().handle(req,res,send); })
```
