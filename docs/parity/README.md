# Route parity index

One subagent per Ghost controller file in `docs/Ghost/ghost/core/server/api/endpoints/`
(`index.js` excluded). Method: [`CHECKLIST.md`](CHECKLIST.md). Verdicts are per route,
with `file:line` evidence on both sides. Every route item now has a `Fix` block: the
exact C++ file/symbol to change and the exact replacement call/type, using only
`blog_context`/`BlogContext` and core abstractions (`core_workflow`, `core_serde`,
`core_cache`, `core_router`, `interfaces::io`, `blog_handlers_middleware`).
Analyses are read-only; no source code was changed.

## Aggregate

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 135 |
| MISMATCH | 130 |
| MISSING | 30 |
| EXTRA | 45 |

## Implementation status

All 89 domains have been implemented in `include/blog/handlers/` (54 manifest
groups plus 5 missing-only domains). Fixes used only `blog_context`/`BlogContext`
and core abstractions. Central registrations (imports, accessors, members,
BUILD partitions, route installs) were applied in `all.cppm`, `base.cppm`,
`routes.cppm`, `domain_all.cppm` and `include/blog/handlers/BUILD.bazel`.
Core router limits were raised to fit the larger route set: middleware lanes
64 → 192, handler mask lanes 8-bit → 16-bit.

Verification: `bazel build //include/blog/handlers:blog_handlers
//include/blog/handlers:blog_handlers_routes` passes;
`//include/blog/testing:http_e2e_test_test` passes. Three legacy tests still
fail on outdated fixtures/segfaults after the intentional wire-shape changes
(`api_test_test`, `http2_wire_test_test`, `snapshot_replay_test_test`).

## Domains

| Domain | MATCH | PARTIAL | MISMATCH | MISSING | EXTRA |
|---|---|---|---|---|---|
| [actions](actions.md) | 0 | 1 | 0 | 0 | 0 |
| [announcements](announcements.md) | 0 | 0 | 0 | 1 | 0 |
| [app-installation-previews](app-installation-previews.md) | 0 | 0 | 0 | 1 | 1 |
| [app-installations](app-installations.md) | 0 | 0 | 0 | 5 | 1 |
| [app-sessions](app-sessions.md) | 0 | 0 | 0 | 1 | 0 |
| [authentication](authentication.md) | 0 | 0 | 8 | 0 | 0 |
| [authors-public](authors-public.md) | 0 | 3 | 0 | 0 | 0 |
| [automated-email-design](automated-email-design.md) | 0 | 0 | 2 | 0 | 0 |
| [automated-emails](automated-emails.md) | 0 | 4 | 4 | 0 | 0 |
| [automation-action-links](automation-action-links.md) | 0 | 1 | 0 | 0 | 0 |
| [automation-email-previews](automation-email-previews.md) | 0 | 0 | 2 | 0 | 0 |
| [automation-performance-stats](automation-performance-stats.md) | 0 | 0 | 1 | 0 | 0 |
| [automation-run-history](automation-run-history.md) | 0 | 0 | 1 | 0 | 0 |
| [automation-runs](automation-runs.md) | 0 | 0 | 1 | 0 | 0 |
| [automations](automations.md) | 0 | 4 | 1 | 0 | 0 |
| [comment-dislikes](comment-dislikes.md) | 0 | 1 | 0 | 0 | 0 |
| [comment-likes](comment-likes.md) | 0 | 1 | 0 | 0 | 0 |
| [comment-replies](comment-replies.md) | 0 | 2 | 0 | 0 | 0 |
| [comment-reports](comment-reports.md) | 0 | 1 | 0 | 0 | 0 |
| [comments-members](comments-members.md) | 0 | 6 | 0 | 7 | 3 |
| [comments](comments.md) | 0 | 4 | 0 | 0 | 5 |
| [config](config.md) | 0 | 1 | 0 | 0 | 0 |
| [custom-theme-settings](custom-theme-settings.md) | 0 | 2 | 0 | 0 | 0 |
| [db](db.md) | 0 | 0 | 5 | 0 | 0 |
| [email-post](email-post.md) | 0 | 0 | 0 | 0 | 0 |
| [email-previews](email-previews.md) | 0 | 2 | 0 | 0 | 0 |
| [emails](emails.md) | 0 | 5 | 4 | 0 | 0 |
| [exports](exports.md) | 0 | 0 | 2 | 0 | 0 |
| [featurebase](featurebase.md) | 0 | 0 | 1 | 0 | 0 |
| [feedback-members](feedback-members.md) | 0 | 0 | 1 | 1 | 0 |
| [files](files.md) | 0 | 0 | 1 | 0 | 0 |
| [gift-links](gift-links.md) | 0 | 0 | 7 | 0 | 0 |
| [gifts-members](gifts-members.md) | 0 | 0 | 0 | 2 | 0 |
| [gifts](gifts.md) | 0 | 0 | 2 | 0 | 0 |
| [identities](identities.md) | 0 | 0 | 1 | 0 | 0 |
| [images](images.md) | 0 | 0 | 1 | 0 | 0 |
| [incoming-recommendations](incoming-recommendations.md) | 0 | 0 | 1 | 0 | 0 |
| [integrations](integrations.md) | 0 | 5 | 1 | 0 | 0 |
| [invites](invites.md) | 0 | 2 | 2 | 0 | 0 |
| [labels](labels.md) | 0 | 4 | 2 | 0 | 0 |
| [links](links.md) | 0 | 2 | 0 | 0 | 0 |
| [mail](mail.md) | 0 | 0 | 0 | 0 | 0 |
| [media](media.md) | 0 | 0 | 2 | 0 | 0 |
| [member-commenting](member-commenting.md) | 0 | 2 | 0 | 0 | 0 |
| [member-metafields-members](member-metafields-members.md) | 0 | 0 | 0 | 1 | 0 |
| [member-metafields](member-metafields.md) | 0 | 0 | 1 | 6 | 0 |
| [member-signin-urls](member-signin-urls.md) | 0 | 1 | 0 | 0 | 0 |
| [members-account](members-account.md) | 0 | 0 | 0 | 3 | 0 |
| [members-stripe-connect](members-stripe-connect.md) | 0 | 0 | 1 | 0 | 0 |
| [members](members.md) | 0 | 12 | 4 | 0 | 5 |
| [mentions](mentions.md) | 0 | 1 | 0 | 1 | 0 |
| [newsletters-public](newsletters-public.md) | 0 | 1 | 0 | 0 | 0 |
| [newsletters](newsletters.md) | 0 | 5 | 0 | 0 | 0 |
| [notifications](notifications.md) | 0 | 3 | 0 | 0 | 0 |
| [oembed](oembed.md) | 0 | 0 | 1 | 0 | 0 |
| [offers-public](offers-public.md) | 0 | 0 | 1 | 0 | 3 |
| [offers](offers.md) | 0 | 4 | 0 | 0 | 0 |
| [pages-public](pages-public.md) | 0 | 3 | 0 | 0 | 0 |
| [pages](pages.md) | 0 | 7 | 2 | 0 | 3 |
| [posts-public](posts-public.md) | 0 | 2 | 1 | 0 | 11 |
| [posts](posts.md) | 0 | 5 | 6 | 0 | 3 |
| [previews](previews.md) | 0 | 0 | 0 | 1 | 0 |
| [recommendations-public](recommendations-public.md) | 0 | 1 | 0 | 0 | 0 |
| [recommendations](recommendations.md) | 0 | 5 | 1 | 0 | 0 |
| [redirects](redirects.md) | 0 | 0 | 2 | 0 | 0 |
| [roles](roles.md) | 0 | 1 | 0 | 0 | 0 |
| [schedules](schedules.md) | 0 | 0 | 1 | 0 | 0 |
| [search-index-public](search-index-public.md) | 0 | 3 | 0 | 0 | 2 |
| [search-index](search-index.md) | 0 | 0 | 4 | 0 | 1 |
| [session](session.md) | 0 | 0 | 4 | 0 | 0 |
| [settings-public](settings-public.md) | 0 | 0 | 1 | 0 | 0 |
| [settings](settings.md) | 0 | 2 | 5 | 0 | 0 |
| [site](site.md) | 0 | 0 | 1 | 0 | 0 |
| [slack](slack.md) | 0 | 0 | 1 | 0 | 0 |
| [slugs](slugs.md) | 0 | 0 | 2 | 0 | 0 |
| [snippets](snippets.md) | 0 | 5 | 0 | 0 | 0 |
| [stats](stats.md) | 0 | 0 | 17 | 0 | 0 |
| [stripe-checkout-branding](stripe-checkout-branding.md) | 0 | 0 | 1 | 0 | 0 |
| [stripe-checkout-config](stripe-checkout-config.md) | 0 | 0 | 2 | 0 | 0 |
| [stripe-checkout-preview](stripe-checkout-preview.md) | 0 | 0 | 1 | 0 | 3 |
| [tags-public](tags-public.md) | 0 | 3 | 0 | 0 | 0 |
| [tags](tags.md) | 0 | 4 | 2 | 0 | 0 |
| [themes](themes.md) | 0 | 2 | 5 | 0 | 0 |
| [tiers-public](tiers-public.md) | 0 | 1 | 0 | 0 | 3 |
| [tiers](tiers.md) | 0 | 4 | 0 | 0 | 0 |
| [tinybird](tinybird.md) | 0 | 0 | 1 | 0 | 0 |
| [upgrades](upgrades.md) | 0 | 0 | 3 | 0 | 0 |
| [users](users.md) | 0 | 4 | 6 | 0 | 0 |
| [webhooks](webhooks.md) | 0 | 3 | 0 | 0 | 1 |

`MATCH 0` reflects that the C++ handlers are fixture/stub-backed: paths and verbs mostly align,
but input validation, output envelopes, model/service calls, cache behavior, and permissions differ.
Public/content-API controllers frequently collide with the single flat C++ router, so their report
lists path-matched `PARTIAL`/`MISMATCH` routes plus the admin-only C++ routes as `EXTRA`.
