# core_workflow

C++20 module workflow abstraction with Medusa semantics, built after the same
pattern as `include/core/router`, `core_database`, `core_cache` and `core_cron`.
The driver and runtime are **not part of the public interface** — callers go
through `WorkflowBuilder` → `WorkflowDefinition` → `Workflow` and never receive
a driver or a backend.

## Shape

`base.cppm` is the primary interface (`export module core_workflow;`). It
re-exports the public partitions. `workflow_driver`, `local_workflow_driver`
and `workflow_runtime` are re-exported only so the toolchain accepts them as
interface partitions; their declarations live in a non-exported namespace, so
an importer of `core_workflow` cannot name them.

Each partition defines one class (or one cohesive value type) and one
`boost.ut` test suite under `CONGELADO_TEST`.

## No macro, no reflection

A step is a plain class. No `CORE_WORKFLOW_STEP`, no ADL hook, no `consteval`
helper. The `WorkflowStep<S>` concept validates the shape at compile time:

```cpp
import core_workflow;

class DeletePromotionsStep
{
public:
    using Input = DeletePromotionsInput;
    using Output = void;
    static constexpr std::string_view name{"delete-promotions"};

    StepResponse<Output> invoke(const Input& in, WorkflowContext& ctx)
    {
        ctx.service<"promotion">().soft_delete(in.ids);
        return StepResponse<Output>{in.ids};
    }

    void compensate(const WorkflowValue& ids, WorkflowContext& ctx)
    {
        ctx.service<"promotion">().restore(
            WorkflowValueAs{}.operator()<std::vector<std::string>>(ids));
    }
};
```

`WorkflowStep<S>` checks `Input`, `Output`, `name`, `invoke` and `compensate`.
Missing `compensate`, wrong signatures or a duplicate name fail at compile time
(or at `build()` for the name check).

## Composition

Medusa's single-pass constructor is a builder. Deferred field access becomes an
explicit typed instance functor — no JS proxies:

```cpp
const auto definition = WorkflowBuilder{}
    .name("delete-promotions")
    .step<DeletePromotionsStep>()
    .step<NotifyStep>(StepField<Promotion, std::string>{&Promotion::id, "delete-promotions"})
    .hook("promotionsDeleted")
    .build();

Workflow workflow;
workflow.register_workflow(definition);
const WorkflowExecution execution = workflow.run(
    "delete-promotions", DeletePromotionsInput{{"prom_01"}});
```

`StepResponse(output, compensateInput?)` defaults compensate input to output.
`permanent_failure()` and `skip()` are carried as `StepSignal` sentinels.
Failure inverts the completed-step list and calls each `compensate` in reverse.

## State model

Enums match Medusa's `TransactionState`, `StepState` and `StepStatus`.
`WorkflowCheckpoint::merge` uses the same `state_flow_order` precedence, where
`DONE` sorts before `WAITING_TO_COMPENSATE`. `WorkflowContext:idempotency_key`
produces `workflow:transaction:step:action`.

## Current scope

`WorkflowEngine::DISTRIBUTED` currently selects the in-process driver. The
capability slot exists so a queue-backed driver can be added without touching
callers. `DO_WHILE`, `FORK_JOIN_DYNAMIC`, scheduler and Redis checkpoint storage
are not implemented yet.

## Build

`BUILD` uses `congelado_module_library` with `base.cppm` as the primary
interface, mirroring `include/core/router/BUILD`.
