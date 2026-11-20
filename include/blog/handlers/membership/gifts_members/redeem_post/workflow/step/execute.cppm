export module blog_handlers:gifts_members_redeem_post_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import :outbox_shim;
import :gifts_members_redeem_get_step;

export namespace blog::handlers::gifts_members::redeem_post::workflow {

class ExecuteStep {
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"gifts_members_redeem_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        namespace support = blog::handlers::gifts_members::support;
        namespace outbox = blog::handlers::outbox;
        auto& ctx = *s_ctx;

        // Ghost: data:['token'], token validated string/required (gifts-members.js:31-39).
        const std::string token = support::token_from_path(in.get_path());
        if (token.empty()) {
            return result("400", support::validation_error("A token is required", "token"));
        }

        // Ghost: frame.options.context.member.id required (gift-controller.ts:34-40).
        // Member auth is NOT the admin bearer gate; identity comes from a header because
        // no C++ loadMemberSession primitive exists.
        const std::string memberId{in.find_header("X-Member-Id")};
        if (memberId.empty()) {
            return result("401", support::ghost_error(
                "UNAUTHORIZED", "UnauthorizedError", "Member authentication required."));
        }
        // Body has no validated fields beyond token (bodyParser.json 50mb on the route).
        (void)in.body_as_string();

        const auto gift = support::find_gift(ctx, token);
        if (!gift) {
            return result("404", support::ghost_error(
                "GIFT_NOT_FOUND", "NotFoundError", "This gift does not exist."));
        }

        // Ghost fetches the member with forUpdate and asserts against member.status —
        // not against a request header (gift-service.ts:908-920).
        const auto member = ctx.store().find_by_resource("members", memberId);
        if (!member) {
            return result("404", support::ghost_error(
                "", "NotFoundError", "Member not found: " + memberId));
        }
        const auto memberRows = blog::api::parse_array("[" + *member + "]");
        const support::Record memberRecord = memberRows.empty() ? support::Record{} : memberRows.front();
        if (const auto rejected = support::redeemability(*gift, support::field(memberRecord, "status"))) {
            return result(rejected->status,
                          support::ghost_error(rejected->code, rejected->type, rejected->message));
        }

        const std::string tierId = support::field(*gift, "tier_id", support::field(*gift, "tierId"));
        const std::string cadence = support::field(*gift, "cadence");
        const std::string duration = support::field(*gift, "duration");
        const std::string redeemedAt = support::now_iso();
        // Gift.redeem derives consumes_at from the redemption time, not the gift's
        // stored expires_at (gift.ts:252-268).
        const std::string consumesAt = support::consumes_at(redeemedAt, cadence, duration);

        // Ghost runs redeemGift inside a Knex transaction. The outbox provides the
        // transaction primitive; store edits are the context-local equivalent of the
        // repository writes (no DB driver in the allowed modules).
        auto& bus = outbox::global_outbox();
        bus.begin_transaction();
        (void)ctx.store().edit("members", memberId,
            "{\"status\":\"gift\",\"products\":[{\"id\":" + support::quote(tierId) +
            ",\"expiry_at\":" + support::quote(consumesAt) + "}]}");
        const std::string giftId = support::field(*gift, "id");
        if (!giftId.empty()) {
            (void)ctx.store().edit("gifts", giftId,
                "{\"status\":\"redeemed\",\"redeemer_member_id\":" + support::quote(memberId) +
                ",\"redeemed_at\":" + support::quote(redeemedAt) +
                ",\"consumes_at\":" + support::quote(consumesAt) + "}");
        }
        cancel_pending_deliveries(ctx, token, giftId);

        // notifyMembershipStartedAutomation(memberStatus:'paid') — gift-service.ts:940-945.
        bus.emit(outbox::Event{"member.subscription.started",
            "{\"member_id\":" + support::quote(memberId) + ",\"member_status\":\"paid\"}",
            memberId});
        bus.commit();

        // Ghost's post-transaction notify(): staff email + reminder scheduler
        // (gift-service.ts:854-888). No external transport in the allowed modules —
        // emit observable outbox events instead of dropping the side effects.
        const auto tier = ctx.store().find_by_resource("tiers", tierId);
        const auto tierRows = tier ? blog::api::parse_array("[" + *tier + "]")
                                   : blog::api::parse_array("[]");
        const support::Record tierRecord = tierRows.empty() ? support::Record{} : tierRows.front();
        bus.emit(outbox::Event{"gift.staff.notify_subscription_started",
            "{\"member_id\":" + support::quote(memberId) +
            ",\"member_email\":" + support::nullable(memberRecord, "email") +
            ",\"member_name\":" + support::nullable(memberRecord, "name") +
            ",\"tier_name\":" + support::nullable(tierRecord, "name") +
            ",\"cadence\":" + support::quote(cadence) +
            ",\"duration\":" + support::quote(duration) +
            ",\"buyer_email\":" + support::nullable(*gift, "buyer_email") + "}",
            token});
        if (const auto due = support::iso_ms(consumesAt)) {
            // GIFT_REMINDER_LEAD_DAYS = 7 (gifts/constants.ts:10).
            const long long reminderDueAt = *due - 7LL * 24 * 60 * 60 * 1000;
            bus.emit(outbox::Event{"gift.reminder.schedule",
                "{\"due_at_ms\":" + std::to_string(reminderDueAt) +
                ",\"gift_token\":" + support::quote(token) + "}",
                token});
        }

        // serializeRedemption runs on the redeemed gift (status/consumes_at updated).
        support::Record redeemed = *gift;
        redeemed["status"] = "redeemed";
        redeemed["redeemer_member_id"] = support::quote(memberId);
        redeemed["redeemed_at"] = support::quote(redeemedAt);
        redeemed["consumes_at"] = support::quote(consumesAt);
        const auto out = support::serialize_redemption(ctx, redeemed, token);
        return result(out.status, out.body);
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static core::workflow::StepResponse<Output> result(std::string status, std::string body)
    {
        return core::workflow::StepResponse<Output>{{{"status", std::move(status)}, {"body", std::move(body)}}};
    }

    static void cancel_pending_deliveries(blog::BlogContext& ctx,
                                          const std::string& token,
                                          const std::string& giftId)
    {
        namespace support = blog::handlers::gifts_members::support;
        for (auto& delivery : blog::api::parse_array(ctx.store().json_for("gift_deliveries"))) {
            const std::string id = support::field(delivery, "id");
            const bool matches = (!token.empty() && support::field(delivery, "gift_token") == token) ||
                                 (!giftId.empty() && support::field(delivery, "gift_id") == giftId);
            if (id.empty() || !matches || support::field(delivery, "status") == "cancelled") {
                continue;
            }
            (void)ctx.store().edit("gift_deliveries", id, "{\"status\":\"cancelled\"}");
        }
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::gifts_members::redeem_post::workflow
