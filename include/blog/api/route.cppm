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
        blog::api::BlogHandlers& handlers
    )
    {
        ActionsRoutes{}.install(router, handlers.actions());
        AppsRoutes{}.install(router, handlers.apps());
        AuthenticationRoutes{}.install(router, handlers.authentication());
        AuthorsRoutes{}.install(router, handlers.authors());
        AutomatedEmailsRoutes{}.install(router, handlers.automated_emails());
        AutomationsRoutes{}.install(router, handlers.automations());
        CommentsRoutes{}.install(router, handlers.comments());
        ConfigRoutes{}.install(router, handlers.config());
        CustomThemeSettingsRoutes{}.install(router, handlers.custom_theme_settings());
        DbRoutes{}.install(router, handlers.db());
        EmailPreviewsRoutes{}.install(router, handlers.email_previews());
        EmailsRoutes{}.install(router, handlers.emails());
        ExportsRoutes{}.install(router, handlers.exports());
        FeaturebaseRoutes{}.install(router, handlers.featurebase());
        FeedbackRoutes{}.install(router, handlers.feedback());
        FilesRoutes{}.install(router, handlers.files());
        GiftLinksRoutes{}.install(router, handlers.gift_links());
        GiftsRoutes{}.install(router, handlers.gifts());
        IdentitiesRoutes{}.install(router, handlers.identities());
        ImagesRoutes{}.install(router, handlers.images());
        IncomingRecommendationsRoutes{}.install(router, handlers.incoming_recommendations());
        IntegrationsRoutes{}.install(router, handlers.integrations());
        InvitesRoutes{}.install(router, handlers.invites());
        LabelsRoutes{}.install(router, handlers.labels());
        LinksRoutes{}.install(router, handlers.links());
        MediaRoutes{}.install(router, handlers.media());
        MembersRoutes{}.install(router, handlers.members());
        MentionsRoutes{}.install(router, handlers.mentions());
        NewslettersRoutes{}.install(router, handlers.newsletters());
        NotificationsRoutes{}.install(router, handlers.notifications());
        OembedRoutes{}.install(router, handlers.oembed());
        OffersRoutes{}.install(router, handlers.offers());
        PagesRoutes{}.install(router, handlers.pages());
        PostsRoutes{}.install(router, handlers.posts(), handlers.posts_browse());
        RecommendationsRoutes{}.install(router, handlers.recommendations());
        RedirectsRoutes{}.install(router, handlers.redirects());
        RolesRoutes{}.install(router, handlers.roles());
        SchedulesRoutes{}.install(router, handlers.schedules());
        SearchIndexRoutes{}.install(router, handlers.search_index());
        SessionRoutes{}.install(router, handlers.session());
        SettingsRoutes{}.install(router, handlers.settings());
        SiteRoutes{}.install(router, handlers.site());
        SlackRoutes{}.install(router, handlers.slack());
        SlugsRoutes{}.install(router, handlers.slugs());
        SnippetsRoutes{}.install(router, handlers.snippets());
        StatsRoutes{}.install(router, handlers.stats());
        StripeRoutes{}.install(router, handlers.stripe());
        TagsRoutes{}.install(router, handlers.tags());
        ThemesRoutes{}.install(router, handlers.themes());
        TiersRoutes{}.install(router, handlers.tiers());
        TinybirdRoutes{}.install(router, handlers.tinybird());
        UpgradesRoutes{}.install(router, handlers.upgrades());
        UsersRoutes{}.install(router, handlers.users());
        WebhooksRoutes{}.install(router, handlers.webhooks());
    }
};

} // namespace blog::api
