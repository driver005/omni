# core_cron

C++20 module cron abstraction, built after the same pattern as
`include/core/router`, `core_database` and `core_cache`. The timing engine and
job table are **not part of the public interface** — callers go through
`CronBuilder` → `Cron` and never receive a backend, a driver or a poll loop.

## Shape

`base.cppm` is the primary interface (`export module core_cron;`). Engine
partitions (`cron_backend`, `local_cron_backend`, `cron_driver`, `cron_runtime`,
`cron_context`) are re-exported only for toolchain correctness; their
declarations are not exported, so an importer cannot name a backend. `Cron`
owns its state through a private incomplete `Runtime` type (pImpl).

Each partition defines one class (or one cohesive value type) and one
`boost.ut` test suite under `CONGELADO_TEST`.

## Cron expression parsing

`CronExpression` parses the standard five fields (`minute hour day month
weekday`) with `*`, `*/N`, `A-B` and comma lists, computes the next fire time
minute-by-minute with a four-year horizon, and can preview the next N runs.

## Compile-time reflection

Plain classes become job definitions with a macro. Name and expression are
compile-time constants used by `cron.query()` via `where_job<Job>()` and
`Cron::schedule<Job>()`.

```cpp
import core_cron;

struct NightlyBackup
{
    static void run() {}
};

CORE_CRON_JOB(NightlyBackup, "nightly-backup", "0 2 * * *");
```

The macro emits an ADL hook (`cron_reflect_meta(NightlyBackup*)`), so it works
at any namespace scope.

## Scheduling, queries and polling

```cpp
core::cron::CronBuilder builder;
auto cron = builder.engine(core::cron::CronEngine::LOCAL)
                  .option("timezone", "UTC")
                  .build();                       // opaque handle

std::vector<std::string> fired;
cron.set_fire_callback([&](std::string_view name) { fired.emplace_back(name); });

auto job = cron.schedule<NightlyBackup>();        // registers by reflected name

auto q = cron.query();                            // compile-time member checks
q.where_name(core::cron::CronCompare::CONTAINS, "backup").limit(5);
auto matches = cron.execute(q.spec());

auto events = cron.poll_due();                    // fire all jobs due now
cron.remove("nightly-backup");
```

`Cron::validate`, `Cron::next_after` and `Cron::next_runs` are pure static
helpers — no backend access required.

## Build

`BUILD` uses `congelado_module_library` with `base.cppm` as the primary
interface, mirroring `include/core/router/BUILD`.
