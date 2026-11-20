# Strict-parity implementation manifest

Groups are keyed by C++ handler folder(s). One agent per group. Shared files (all.cppm, base.cppm, central routes.cppm, BUILD.bazel, middleware/base.cppm, core modules) are owned by the main agent.

| # | C++ folders | Behavior reports |
|---|---|---|
| 1 | `include/blog/handlers/admin/authors` | `authors-public.md` |
| 2 | `include/blog/handlers/admin/labels` | `labels.md` |
| 3 | `include/blog/handlers/admin/notifications` | `notifications.md` |
| 4 | `include/blog/handlers/admin/pages` | `pages-public.md`, `pages.md` |
| 5 | `include/blog/handlers/admin/pages`, `include/blog/handlers/admin/posts`, `include/blog/handlers/membership/gift_links` | `gift-links.md` |
| 6 | `include/blog/handlers/admin/posts` | `posts-public.md`, `posts.md` |
| 7 | `include/blog/handlers/admin/previews` | `previews.md` |
| 8 | `include/blog/handlers/admin/snippets` | `snippets.md` |
| 9 | `include/blog/handlers/admin/tags` | `tags-public.md`, `tags.md` |
| 10 | `include/blog/handlers/admin/users` | `users.md` |
| 11 | `include/blog/handlers/auth/authentication` | `authentication.md` |
| 12 | `include/blog/handlers/auth/identities` | `identities.md` |
| 13 | `include/blog/handlers/auth/invites` | `invites.md` |
| 14 | `include/blog/handlers/auth/session` | `session.md` |
| 15 | `include/blog/handlers/discovery/comments` | `comment-dislikes.md`, `comment-likes.md`, `comment-replies.md`, `comment-reports.md`, `comments-members.md`, `comments.md` |
| 16 | `include/blog/handlers/discovery/feedback` | `feedback-members.md` |
| 17 | `include/blog/handlers/discovery/incoming_recommendations` | `incoming-recommendations.md` |
| 18 | `include/blog/handlers/discovery/mentions` | `mentions.md` |
| 19 | `include/blog/handlers/discovery/recommendations` | `recommendations-public.md`, `recommendations.md` |
| 20 | `include/blog/handlers/discovery/search_index` | `search-index-public.md`, `search-index.md` |
| 21 | `include/blog/handlers/external/actions` | `actions.md` |
| 22 | `include/blog/handlers/external/app_sessions`, `include/blog/handlers/external/apps` | `app-sessions.md` |
| 23 | `include/blog/handlers/external/apps` | `app-installation-previews.md`, `app-installations.md` |
| 24 | `include/blog/handlers/external/featurebase` | `featurebase.md` |
| 25 | `include/blog/handlers/external/featurebase`, `include/blog/handlers/system/config` | `config.md` |
| 26 | `include/blog/handlers/external/integrations` | `integrations.md` |
| 27 | `include/blog/handlers/external/links` | `links.md` |
| 28 | `include/blog/handlers/external/oembed` | `oembed.md` |
| 29 | `include/blog/handlers/external/slack` | `slack.md` |
| 30 | `include/blog/handlers/external/stripe` | `stripe-checkout-branding.md`, `stripe-checkout-config.md`, `stripe-checkout-preview.md` |
| 31 | `include/blog/handlers/external/tinybird` | `tinybird.md` |
| 32 | `include/blog/handlers/external/upgrades` | `upgrades.md` |
| 33 | `include/blog/handlers/external/webhooks` | `webhooks.md` |
| 34 | `include/blog/handlers/membership/announcements` | `announcements.md` |
| 35 | `include/blog/handlers/membership/automated_emails` | `automated-email-design.md`, `automated-emails.md` |
| 36 | `include/blog/handlers/membership/automations` | `automation-action-links.md`, `automation-email-previews.md`, `automation-performance-stats.md`, `automation-run-history.md`, `automation-runs.md`, `automations.md` |
| 37 | `include/blog/handlers/membership/email_previews` | `email-previews.md` |
| 38 | `include/blog/handlers/membership/emails` | `emails.md` |
| 39 | `include/blog/handlers/membership/gifts` | `gifts.md` |
| 40 | `include/blog/handlers/membership/gifts_members` | `gifts-members.md` |
| 41 | `include/blog/handlers/membership/member_metafields_members` | `member-metafields-members.md` |
| 42 | `include/blog/handlers/membership/member_metafields_members`, `include/blog/handlers/membership/members` | `member-metafields.md` |
| 43 | `include/blog/handlers/membership/members` | `member-commenting.md`, `member-signin-urls.md`, `members-account.md`, `members-stripe-connect.md`, `members.md` |
| 44 | `include/blog/handlers/membership/newsletters` | `newsletters-public.md`, `newsletters.md` |
| 45 | `include/blog/handlers/membership/offers` | `offers-public.md`, `offers.md` |
| 46 | `include/blog/handlers/membership/tiers` | `tiers-public.md`, `tiers.md` |
| 47 | `include/blog/handlers/system/custom_theme_settings` | `custom-theme-settings.md` |
| 48 | `include/blog/handlers/system/db` | `db.md` |
| 49 | `include/blog/handlers/system/exports` | `exports.md` |
| 50 | `include/blog/handlers/system/files` | `files.md` |
| 51 | `include/blog/handlers/system/images` | `images.md` |
| 52 | `include/blog/handlers/system/media` | `media.md` |
| 53 | `include/blog/handlers/system/redirects` | `redirects.md` |
| 54 | `include/blog/handlers/system/roles` | `roles.md` |
| 55 | `include/blog/handlers/system/schedules` | `schedules.md` |
| 56 | `include/blog/handlers/system/settings` | `settings-public.md`, `settings.md` |
| 57 | `include/blog/handlers/system/site` | `site.md` |
| 58 | `include/blog/handlers/system/slugs` | `slugs.md` |
| 59 | `include/blog/handlers/system/stats` | `stats.md` |
| 60 | `include/blog/handlers/system/themes` | `themes.md` |
