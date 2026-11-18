export module blog_handlers_routes;

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
        blog::api::BlogHandlers& handlers
    )
    {
        ActionsRoutes{}.install(router, handlers);
        AppsRoutes{}.install(router, handlers);
        AuthenticationRoutes{}.install(router, handlers);
        AuthorsRoutes{}.install(router, handlers);
        AutomatedEmailsRoutes{}.install(router, handlers);
        AutomationsRoutes{}.install(router, handlers);
        CommentsRoutes{}.install(router, handlers);
        ConfigRoutes{}.install(router, handlers);
        CustomThemeSettingsRoutes{}.install(router, handlers);
        DbRoutes{}.install(router, handlers);
        EmailPreviewsRoutes{}.install(router, handlers);
        EmailsRoutes{}.install(router, handlers);
        ExportsRoutes{}.install(router, handlers);
        FeaturebaseRoutes{}.install(router, handlers);
        FeedbackRoutes{}.install(router, handlers);
        FilesRoutes{}.install(router, handlers);
        GiftLinksRoutes{}.install(router, handlers);
        GiftsRoutes{}.install(router, handlers);
        IdentitiesRoutes{}.install(router, handlers);
        ImagesRoutes{}.install(router, handlers);
        IncomingRecommendationsRoutes{}.install(router, handlers);
        IntegrationsRoutes{}.install(router, handlers);
        InvitesRoutes{}.install(router, handlers);
        LabelsRoutes{}.install(router, handlers);
        LinksRoutes{}.install(router, handlers);
        MediaRoutes{}.install(router, handlers);
        MembersRoutes{}.install(router, handlers);
        MentionsRoutes{}.install(router, handlers);
        NewslettersRoutes{}.install(router, handlers);
        NotificationsRoutes{}.install(router, handlers);
        OembedRoutes{}.install(router, handlers);
        OffersRoutes{}.install(router, handlers);
        PagesRoutes{}.install(router, handlers);
        PostsRoutes{}.install(router, handlers);
        RecommendationsRoutes{}.install(router, handlers);
        RedirectsRoutes{}.install(router, handlers);
        RolesRoutes{}.install(router, handlers);
        SchedulesRoutes{}.install(router, handlers);
        SearchIndexRoutes{}.install(router, handlers);
        SessionRoutes{}.install(router, handlers);
        SettingsRoutes{}.install(router, handlers);
        SiteRoutes{}.install(router, handlers);
        SlackRoutes{}.install(router, handlers);
        SlugsRoutes{}.install(router, handlers);
        SnippetsRoutes{}.install(router, handlers);
        StatsRoutes{}.install(router, handlers);
        StripeRoutes{}.install(router, handlers);
        TagsRoutes{}.install(router, handlers);
        ThemesRoutes{}.install(router, handlers);
        TiersRoutes{}.install(router, handlers);
        TinybirdRoutes{}.install(router, handlers);
        UpgradesRoutes{}.install(router, handlers);
        UsersRoutes{}.install(router, handlers);
        WebhooksRoutes{}.install(router, handlers);
    }
};

} // namespace blog::api
