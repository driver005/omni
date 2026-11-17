export module blog_api;

export import :actions_routes;
export import :apps_routes;
export import :authentication_routes;
export import :authors_routes;
export import :automated_emails_routes;
export import :automations_routes;
export import :comments_routes;
export import :config_routes;
export import :custom_theme_settings_routes;
export import :db_routes;
export import :email_previews_routes;
export import :emails_routes;
export import :exports_routes;
export import :featurebase_routes;
export import :feedback_routes;
export import :files_routes;
export import :gift_links_routes;
export import :gifts_routes;
export import :identities_routes;
export import :images_routes;
export import :incoming_recommendations_routes;
export import :integrations_routes;
export import :invites_routes;
export import :labels_routes;
export import :links_routes;
export import :media_routes;
export import :members_routes;
export import :mentions_routes;
export import :newsletters_routes;
export import :notifications_routes;
export import :oembed_routes;
export import :offers_routes;
export import :pages_routes;
export import :posts_routes;
export import :recommendations_routes;
export import :redirects_routes;
export import :roles_routes;
export import :schedules_routes;
export import :search_index_routes;
export import :session_routes;
export import :settings_routes;
export import :site_routes;
export import :slack_routes;
export import :slugs_routes;
export import :snippets_routes;
export import :stats_routes;
export import :stripe_routes;
export import :tags_routes;
export import :themes_routes;
export import :tiers_routes;
export import :tinybird_routes;
export import :upgrades_routes;
export import :users_routes;
export import :webhooks_routes;

import std;
import interfaces;
import core_router;
import blog_handlers;

export namespace blog::api {

/** @brief Installs every Ghost route into a core_router context. */
class ApiRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::CoreHandler& handler
    )
    {
        ActionsRoutes{}.install(router, handler);
        AppsRoutes{}.install(router, handler);
        AuthenticationRoutes{}.install(router, handler);
        AuthorsRoutes{}.install(router, handler);
        AutomatedEmailsRoutes{}.install(router, handler);
        AutomationsRoutes{}.install(router, handler);
        CommentsRoutes{}.install(router, handler);
        ConfigRoutes{}.install(router, handler);
        CustomThemeSettingsRoutes{}.install(router, handler);
        DbRoutes{}.install(router, handler);
        EmailPreviewsRoutes{}.install(router, handler);
        EmailsRoutes{}.install(router, handler);
        ExportsRoutes{}.install(router, handler);
        FeaturebaseRoutes{}.install(router, handler);
        FeedbackRoutes{}.install(router, handler);
        FilesRoutes{}.install(router, handler);
        GiftLinksRoutes{}.install(router, handler);
        GiftsRoutes{}.install(router, handler);
        IdentitiesRoutes{}.install(router, handler);
        ImagesRoutes{}.install(router, handler);
        IncomingRecommendationsRoutes{}.install(router, handler);
        IntegrationsRoutes{}.install(router, handler);
        InvitesRoutes{}.install(router, handler);
        LabelsRoutes{}.install(router, handler);
        LinksRoutes{}.install(router, handler);
        MediaRoutes{}.install(router, handler);
        MembersRoutes{}.install(router, handler);
        MentionsRoutes{}.install(router, handler);
        NewslettersRoutes{}.install(router, handler);
        NotificationsRoutes{}.install(router, handler);
        OembedRoutes{}.install(router, handler);
        OffersRoutes{}.install(router, handler);
        PagesRoutes{}.install(router, handler);
        PostsRoutes{}.install(router, handler);
        RecommendationsRoutes{}.install(router, handler);
        RedirectsRoutes{}.install(router, handler);
        RolesRoutes{}.install(router, handler);
        SchedulesRoutes{}.install(router, handler);
        SearchIndexRoutes{}.install(router, handler);
        SessionRoutes{}.install(router, handler);
        SettingsRoutes{}.install(router, handler);
        SiteRoutes{}.install(router, handler);
        SlackRoutes{}.install(router, handler);
        SlugsRoutes{}.install(router, handler);
        SnippetsRoutes{}.install(router, handler);
        StatsRoutes{}.install(router, handler);
        StripeRoutes{}.install(router, handler);
        TagsRoutes{}.install(router, handler);
        ThemesRoutes{}.install(router, handler);
        TiersRoutes{}.install(router, handler);
        TinybirdRoutes{}.install(router, handler);
        UpgradesRoutes{}.install(router, handler);
        UsersRoutes{}.install(router, handler);
        WebhooksRoutes{}.install(router, handler);
    }
};

} // namespace blog::api
