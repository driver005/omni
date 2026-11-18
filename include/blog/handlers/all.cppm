export module blog_handlers:all;

import std;
import blog_context;
import :handler;
import :special;
import :stripe;
import :actions_handler;
import :apps_handler;
import :authentication_handler;
import :authors_handler;
import :automated_emails_handler;
import :automations_handler;
import :comments_handler;
import :config_handler;
import :custom_theme_settings_handler;
import :db_handler;
import :email_previews_handler;
import :emails_handler;
import :exports_handler;
import :featurebase_handler;
import :feedback_handler;
import :files_handler;
import :gift_links_handler;
import :gifts_handler;
import :identities_handler;
import :images_handler;
import :incoming_recommendations_handler;
import :integrations_handler;
import :invites_handler;
import :labels_handler;
import :links_handler;
import :media_handler;
import :members_handler;
import :mentions_handler;
import :newsletters_handler;
import :notifications_handler;
import :oembed_handler;
import :offers_handler;
import :pages_handler;
import :posts_handler;
import :recommendations_handler;
import :redirects_handler;
import :roles_handler;
import :schedules_handler;
import :search_index_handler;
import :session_handler;
import :settings_handler;
import :site_handler;
import :slack_handler;
import :slugs_handler;
import :snippets_handler;
import :stats_handler;
import :stripe_handler;
import :tags_handler;
import :themes_handler;
import :tiers_handler;
import :tinybird_handler;
import :upgrades_handler;
import :users_handler;
import :webhooks_handler;

export namespace blog::api {

/** @brief Owns the shared core handler plus one domain handler per resource. */
class BlogHandlers
{
public:
    /** @brief Owns a fresh context (tests / standalone use). */
    BlogHandlers() :
        m_owned_context{std::make_unique<blog::BlogContext>()},
        m_ctx{*m_owned_context},
        m_special{},
        m_core{m_ctx, m_special, m_stripe}
    {
    }

    /** @brief Use an injected, externally-owned context (plugin load). */
    explicit BlogHandlers(blog::BlogContext& context) :
        m_ctx{context},
        m_special{},
        m_core{m_ctx, m_special, m_stripe}
    {
    }

    [[nodiscard]] blog::BlogContext& context() noexcept
    {
        return m_ctx;
    }

    [[nodiscard]] CoreHandler& core() noexcept
    {
        return m_core;
    }

    [[nodiscard]] ActionsRouteHandler& actions() noexcept
    {
        return ActionsRouteHandler;
    }
    [[nodiscard]] AppsRouteHandler& apps() noexcept
    {
        return AppsRouteHandler;
    }
    [[nodiscard]] AuthenticationRouteHandler& authentication() noexcept
    {
        return AuthenticationRouteHandler;
    }
    [[nodiscard]] AuthorsRouteHandler& authors() noexcept
    {
        return AuthorsRouteHandler;
    }
    [[nodiscard]] AutomatedEmailsRouteHandler& automated_emails() noexcept
    {
        return AutomatedEmailsRouteHandler;
    }
    [[nodiscard]] AutomationsRouteHandler& automations() noexcept
    {
        return AutomationsRouteHandler;
    }
    [[nodiscard]] CommentsRouteHandler& comments() noexcept
    {
        return CommentsRouteHandler;
    }
    [[nodiscard]] ConfigRouteHandler& config() noexcept
    {
        return ConfigRouteHandler;
    }
    [[nodiscard]] CustomThemeSettingsRouteHandler& custom_theme_settings() noexcept
    {
        return CustomThemeSettingsRouteHandler;
    }
    [[nodiscard]] DbRouteHandler& db() noexcept
    {
        return DbRouteHandler;
    }
    [[nodiscard]] EmailPreviewsRouteHandler& email_previews() noexcept
    {
        return EmailPreviewsRouteHandler;
    }
    [[nodiscard]] EmailsRouteHandler& emails() noexcept
    {
        return EmailsRouteHandler;
    }
    [[nodiscard]] ExportsRouteHandler& exports() noexcept
    {
        return ExportsRouteHandler;
    }
    [[nodiscard]] FeaturebaseRouteHandler& featurebase() noexcept
    {
        return FeaturebaseRouteHandler;
    }
    [[nodiscard]] FeedbackRouteHandler& feedback() noexcept
    {
        return FeedbackRouteHandler;
    }
    [[nodiscard]] FilesRouteHandler& files() noexcept
    {
        return FilesRouteHandler;
    }
    [[nodiscard]] GiftLinksRouteHandler& gift_links() noexcept
    {
        return GiftLinksRouteHandler;
    }
    [[nodiscard]] GiftsRouteHandler& gifts() noexcept
    {
        return GiftsRouteHandler;
    }
    [[nodiscard]] IdentitiesRouteHandler& identities() noexcept
    {
        return IdentitiesRouteHandler;
    }
    [[nodiscard]] ImagesRouteHandler& images() noexcept
    {
        return ImagesRouteHandler;
    }
    [[nodiscard]] IncomingRecommendationsRouteHandler& incoming_recommendations() noexcept
    {
        return IncomingRecommendationsRouteHandler;
    }
    [[nodiscard]] IntegrationsRouteHandler& integrations() noexcept
    {
        return IntegrationsRouteHandler;
    }
    [[nodiscard]] InvitesRouteHandler& invites() noexcept
    {
        return InvitesRouteHandler;
    }
    [[nodiscard]] LabelsRouteHandler& labels() noexcept
    {
        return LabelsRouteHandler;
    }
    [[nodiscard]] LinksRouteHandler& links() noexcept
    {
        return LinksRouteHandler;
    }
    [[nodiscard]] MediaRouteHandler& media() noexcept
    {
        return MediaRouteHandler;
    }
    [[nodiscard]] MembersRouteHandler& members() noexcept
    {
        return MembersRouteHandler;
    }
    [[nodiscard]] MentionsRouteHandler& mentions() noexcept
    {
        return MentionsRouteHandler;
    }
    [[nodiscard]] NewslettersRouteHandler& newsletters() noexcept
    {
        return NewslettersRouteHandler;
    }
    [[nodiscard]] NotificationsRouteHandler& notifications() noexcept
    {
        return NotificationsRouteHandler;
    }
    [[nodiscard]] OembedRouteHandler& oembed() noexcept
    {
        return OembedRouteHandler;
    }
    [[nodiscard]] OffersRouteHandler& offers() noexcept
    {
        return OffersRouteHandler;
    }
    [[nodiscard]] PagesRouteHandler& pages() noexcept
    {
        return PagesRouteHandler;
    }
    [[nodiscard]] PostsRouteHandler& posts() noexcept
    {
        return PostsRouteHandler;
    }
    [[nodiscard]] RecommendationsRouteHandler& recommendations() noexcept
    {
        return RecommendationsRouteHandler;
    }
    [[nodiscard]] RedirectsRouteHandler& redirects() noexcept
    {
        return RedirectsRouteHandler;
    }
    [[nodiscard]] RolesRouteHandler& roles() noexcept
    {
        return RolesRouteHandler;
    }
    [[nodiscard]] SchedulesRouteHandler& schedules() noexcept
    {
        return SchedulesRouteHandler;
    }
    [[nodiscard]] SearchIndexRouteHandler& search_index() noexcept
    {
        return SearchIndexRouteHandler;
    }
    [[nodiscard]] SessionRouteHandler& session() noexcept
    {
        return SessionRouteHandler;
    }
    [[nodiscard]] SettingsRouteHandler& settings() noexcept
    {
        return SettingsRouteHandler;
    }
    [[nodiscard]] SiteRouteHandler& site() noexcept
    {
        return SiteRouteHandler;
    }
    [[nodiscard]] SlackRouteHandler& slack() noexcept
    {
        return SlackRouteHandler;
    }
    [[nodiscard]] SlugsRouteHandler& slugs() noexcept
    {
        return SlugsRouteHandler;
    }
    [[nodiscard]] SnippetsRouteHandler& snippets() noexcept
    {
        return SnippetsRouteHandler;
    }
    [[nodiscard]] StatsRouteHandler& stats() noexcept
    {
        return StatsRouteHandler;
    }
    [[nodiscard]] StripeRouteHandler& stripe() noexcept
    {
        return StripeRouteHandler;
    }
    [[nodiscard]] TagsRouteHandler& tags() noexcept
    {
        return TagsRouteHandler;
    }
    [[nodiscard]] ThemesRouteHandler& themes() noexcept
    {
        return ThemesRouteHandler;
    }
    [[nodiscard]] TiersRouteHandler& tiers() noexcept
    {
        return TiersRouteHandler;
    }
    [[nodiscard]] TinybirdRouteHandler& tinybird() noexcept
    {
        return TinybirdRouteHandler;
    }
    [[nodiscard]] UpgradesRouteHandler& upgrades() noexcept
    {
        return UpgradesRouteHandler;
    }
    [[nodiscard]] UsersRouteHandler& users() noexcept
    {
        return UsersRouteHandler;
    }
    [[nodiscard]] WebhooksRouteHandler& webhooks() noexcept
    {
        return WebhooksRouteHandler;
    }

private:
    std::unique_ptr<blog::BlogContext> m_owned_context{};
    blog::BlogContext& m_ctx;
    SpecialEndpoints m_special{};
    StripeHandler m_stripe{};
    CoreHandler m_core;
        ActionsRouteHandler ActionsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        AppsRouteHandler AppsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        AuthenticationRouteHandler AuthenticationRouteHandler{m_ctx, m_core, m_special, m_stripe};
        AuthorsRouteHandler AuthorsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        AutomatedEmailsRouteHandler AutomatedEmailsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        AutomationsRouteHandler AutomationsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        CommentsRouteHandler CommentsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        ConfigRouteHandler ConfigRouteHandler{m_ctx, m_core, m_special, m_stripe};
        CustomThemeSettingsRouteHandler CustomThemeSettingsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        DbRouteHandler DbRouteHandler{m_ctx, m_core, m_special, m_stripe};
        EmailPreviewsRouteHandler EmailPreviewsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        EmailsRouteHandler EmailsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        ExportsRouteHandler ExportsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        FeaturebaseRouteHandler FeaturebaseRouteHandler{m_ctx, m_core, m_special, m_stripe};
        FeedbackRouteHandler FeedbackRouteHandler{m_ctx, m_core, m_special, m_stripe};
        FilesRouteHandler FilesRouteHandler{m_ctx, m_core, m_special, m_stripe};
        GiftLinksRouteHandler GiftLinksRouteHandler{m_ctx, m_core, m_special, m_stripe};
        GiftsRouteHandler GiftsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        IdentitiesRouteHandler IdentitiesRouteHandler{m_ctx, m_core, m_special, m_stripe};
        ImagesRouteHandler ImagesRouteHandler{m_ctx, m_core, m_special, m_stripe};
        IncomingRecommendationsRouteHandler IncomingRecommendationsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        IntegrationsRouteHandler IntegrationsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        InvitesRouteHandler InvitesRouteHandler{m_ctx, m_core, m_special, m_stripe};
        LabelsRouteHandler LabelsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        LinksRouteHandler LinksRouteHandler{m_ctx, m_core, m_special, m_stripe};
        MediaRouteHandler MediaRouteHandler{m_ctx, m_core, m_special, m_stripe};
        MembersRouteHandler MembersRouteHandler{m_ctx, m_core, m_special, m_stripe};
        MentionsRouteHandler MentionsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        NewslettersRouteHandler NewslettersRouteHandler{m_ctx, m_core, m_special, m_stripe};
        NotificationsRouteHandler NotificationsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        OembedRouteHandler OembedRouteHandler{m_ctx, m_core, m_special, m_stripe};
        OffersRouteHandler OffersRouteHandler{m_ctx, m_core, m_special, m_stripe};
        PagesRouteHandler PagesRouteHandler{m_ctx, m_core, m_special, m_stripe};
        PostsRouteHandler PostsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        RecommendationsRouteHandler RecommendationsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        RedirectsRouteHandler RedirectsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        RolesRouteHandler RolesRouteHandler{m_ctx, m_core, m_special, m_stripe};
        SchedulesRouteHandler SchedulesRouteHandler{m_ctx, m_core, m_special, m_stripe};
        SearchIndexRouteHandler SearchIndexRouteHandler{m_ctx, m_core, m_special, m_stripe};
        SessionRouteHandler SessionRouteHandler{m_ctx, m_core, m_special, m_stripe};
        SettingsRouteHandler SettingsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        SiteRouteHandler SiteRouteHandler{m_ctx, m_core, m_special, m_stripe};
        SlackRouteHandler SlackRouteHandler{m_ctx, m_core, m_special, m_stripe};
        SlugsRouteHandler SlugsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        SnippetsRouteHandler SnippetsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        StatsRouteHandler StatsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        StripeRouteHandler StripeRouteHandler{m_ctx, m_core, m_special, m_stripe};
        TagsRouteHandler TagsRouteHandler{m_ctx, m_core, m_special, m_stripe};
        ThemesRouteHandler ThemesRouteHandler{m_ctx, m_core, m_special, m_stripe};
        TiersRouteHandler TiersRouteHandler{m_ctx, m_core, m_special, m_stripe};
        TinybirdRouteHandler TinybirdRouteHandler{m_ctx, m_core, m_special, m_stripe};
        UpgradesRouteHandler UpgradesRouteHandler{m_ctx, m_core, m_special, m_stripe};
        UsersRouteHandler UsersRouteHandler{m_ctx, m_core, m_special, m_stripe};
        WebhooksRouteHandler WebhooksRouteHandler{m_ctx, m_core, m_special, m_stripe};
};

} // namespace blog::api
