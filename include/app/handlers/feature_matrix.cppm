module;
#include <cstdio>
export module app_handlers:feature_matrix;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_connector;
import app_metrics;
import app_store;
import app_services;

export namespace app::handlers::feature_matrix {

/**
 * Static capability data transcribed from upstream Hyperswitch at SHA
 * 7736d55adaf235f7af74e133ae408573b1826823. Each catalog entry is the combined equivalent of the
 * per-connector static tables consumed by `build_connector_feature_details`
 * (routes/feature_matrix.rs:75-112):
 *   - `ConnectorInfo`                 <- get_connector_about()   (router_response_types.rs:815-824)
 *   - `SupportedPaymentMethods`       <- get_supported_payment_methods() (router_response_types.rs:812)
 *   - `[EventClass]`                  <- get_supported_webhook_flows()
 * Wire strings are the serde snake_case forms from common_enums/src/enums.rs and the
 * `PaymentMethodType::to_display_name()` mapping (enums.rs:2886).
 */
namespace detail {

// GENERATED from upstream Hyperswitch @ 7736d55adaf235f7af74e133ae408573b1826823
// connector static capability tables (hyperswitch_connectors + common_enums).
namespace catalog {
struct FeatureRow { std::string_view pm, pmt, display, mandates, refunds;
  std::vector<std::string_view> captures; bool has_card{false};
  std::string_view three_ds, no_three_ds; std::vector<std::string_view> networks; };
struct Entry { std::string_view display_name, description, category, integration_status;
  std::vector<std::string_view> webhooks; std::vector<FeatureRow> methods; };
inline const std::map<std::string, Entry> kEntries{
  {"absa_sanlam", {
    "AbsaSanlam", "AbsaSanlam connector", "payment_gateway", "live",
    {},
    {
      {"bank_debit","eft_debit_order","EFT Debit Order","not_supported","not_supported",
       {"automatic"},false,"","",{}},
    }
  }},
  {"aci", {
    "ACI", "ACI Payments delivers secure, real-time electronic payment solutions for businesses, banks, and governments, enabling seamless transactions across channels.", "payment_gateway", "sandbox",
    {},
    {
      {"wallet","mb_way","MB WAY","not_supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"wallet","ali_pay","Alipay","not_supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual"},true,"supported","supported",{"visa","mastercard","american_express","jcb","diners_club","discover","union_pay","maestro"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual"},true,"supported","supported",{"visa","mastercard","american_express","jcb","diners_club","discover","union_pay","maestro"}},
      {"bank_redirect","eps","EPS","not_supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"bank_redirect","eft","EFT","not_supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"bank_redirect","ideal","iDEAL","not_supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"bank_redirect","giropay","Giropay","not_supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"bank_redirect","sofort","Sofort","not_supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"bank_redirect","interac","Interac","not_supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"bank_redirect","przelewy24","Przelewy24","not_supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"bank_redirect","trustly","Trustly","not_supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"pay_later","klarna","Klarna","not_supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","not_supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"wallet","google_pay","Google Pay","not_supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"wallet","samsung_pay","Samsung Pay","not_supported","supported",
       {},false,"","",{}},
    }
  }},
  {"adyen", {
    "Adyen", "Adyen is a Dutch payment company with the status of an acquiring bank that allows businesses to accept e-commerce, mobile, and point-of-sale payments. It is listed on the stock exchange Euronext Amsterdam", "payment_gateway", "live",
    {"payments","refunds","disputes","payouts","mandates"},
    {}
  }},
  {"adyenplatform", {
    "Adyen Platform", "Adyen Platform for marketplace payouts and disbursements", "payout_processor", "sandbox",
    {},
    {}
  }},
  {"affirm", {
    "Affirm", "Affirm connector is a payment gateway integration that processes Affirm's buy now, pay later financing by managing payment authorization, capture, refunds, and transaction sync via Affirm’s API.", "payment_gateway", "beta",
    {},
    {
      {"pay_later","affirm","Affirm","not_supported","supported",
       {},false,"","",{}},
    }
  }},
  {"airwallex", {
    "Airwallex", "Airwallex is a multinational financial technology company offering financial services and software as a service (SaaS)", "payment_gateway", "sandbox",
    {"payments","refunds","disputes"},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","jcb","diners_club","union_pay","discover"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","jcb","diners_club","union_pay","discover"}},
      {"wallet","google_pay","Google Pay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","paypal","PayPal","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"wallet","skrill","Skrill","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"pay_later","klarna","Klarna","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"pay_later","atome","Atome","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_redirect","trustly","Trustly","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_redirect","blik","BLIK","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_redirect","ideal","iDEAL","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_transfer","indonesian_bank_transfer","Indonesian Bank Transfer","not_supported","not_supported",
       {"automatic","sequential_automatic"},false,"","",{}},
    }
  }},
  {"amazonpay", {
    "Amazon Pay", "Amazon Pay is an Alternative Payment Method (APM) connector that allows merchants to accept payments using customers' stored Amazon account details, providing a seamless checkout experience.", "alternative_payment_method", "beta",
    {},
    {
      {"wallet","amazon_pay","Amazon Pay","not_supported","supported",
       {"automatic"},false,"","",{}},
    }
  }},
  {"archipel", {
    "Archipel", "Full-service processor offering secure payment solutions and innovative banking technologies for businesses of all sizes.", "payment_gateway", "live",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"mastercard","visa","american_express","diners_club","discover","cartes_bancaires"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"mastercard","visa","american_express","diners_club","discover","cartes_bancaires"}},
      {"wallet","apple_pay","Apple Pay","not_supported","supported",
       {},false,"","",{}},
    }
  }},
  {"authipay", {
    "Authipay", "Authipay is a Fiserv-powered payment gateway for the EMEA region supporting Visa and Mastercard transactions. Features include flexible capture methods (automatic, manual, sequential), partial captures/refunds, payment tokenization, and secure HMAC SHA256 authentication.", "payment_gateway", "sandbox",
    {},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","sequential_automatic","manual"},true,"not_supported","supported",{"visa","mastercard"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","sequential_automatic","manual"},true,"not_supported","supported",{"visa","mastercard"}},
    }
  }},
  {"authorizedotnet", {
    "Authorize.net", " Authorize.net supports payment processing by helping small businesses accept credit card and eCheck payments online, in person on the go.", "payment_gateway", "live",
    {"payments","refunds"},
    {
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","paypal","PayPal","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"american_express","discover","jcb","mastercard","visa"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"american_express","discover","jcb","mastercard","visa"}},
    }
  }},
  {"bambora", {
    "Bambora", "Bambora is a leading online payment provider in Canada and United States.", "payment_gateway", "sandbox",
    {},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","discover","jcb","diners_club"}},
    }
  }},
  {"bamboraapac", {
    "Bambora Asia-Pacific", "Bambora Asia-Pacific, provides comprehensive payment solutions, offering merchants smart and smooth payment processing capabilities.", "payment_gateway", "sandbox",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"visa","mastercard","diners_club","interac","american_express","jcb","discover","cartes_bancaires","union_pay"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"visa","mastercard","diners_club","interac","american_express","jcb","discover","cartes_bancaires","union_pay"}},
    }
  }},
  {"bankofamerica", {
    "Bank Of America", "It is the second-largest banking institution in the United States and the second-largest bank in the world by market capitalization ", "bank_acquirer", "live",
    {},
    {
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","samsung_pay","Samsung Pay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"visa","mastercard","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay","maestro","interac"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"visa","mastercard","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay","maestro","interac"}},
    }
  }},
  {"barclaycard", {
    "BarclayCard SmartPay Fuse", "Barclaycard, part of Barclays Bank UK PLC, is a leading global payment business that helps consumers, retailers and businesses to make and take payments flexibly, and to access short-term credit and point of sale finance.", "bank_acquirer", "sandbox",
    {},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"mastercard","visa","american_express","jcb","discover","maestro","interac","diners_club","cartes_bancaires","union_pay"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"mastercard","visa","american_express","jcb","discover","maestro","interac","diners_club","cartes_bancaires","union_pay"}},
      {"wallet","google_pay","Google Pay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","not_supported","supported",
       {},false,"","",{}},
    }
  }},
  {"betterpayment", {
    "Betterpayment", "Better Payment Germany GmbH (Deutsche Bank Group) — Wero digital wallet payments", "payment_gateway", "beta",
    {},
    {
      {"wallet","wero","Wero","not_supported","supported",
       {"automatic"},false,"","",{}},
    }
  }},
  {"billwerk", {
    "Billwerk", "Billwerk+ Pay is an acquirer independent payment gateway that's easy to setup with more than 50 recurring and non-recurring payment methods.", "payment_gateway", "sandbox",
    {},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"mastercard","visa","american_express","discover","jcb","union_pay","diners_club","interac","cartes_bancaires"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"mastercard","visa","american_express","discover","jcb","union_pay","diners_club","interac","cartes_bancaires"}},
    }
  }},
  {"bitpay", {
    "Bitpay", "BitPay is a cryptocurrency payment processor that enables businesses to accept Bitcoin and other digital currencies ", "alternative_payment_method", "sandbox",
    {"payments"},
    {
      {"crypto","crypto_currency","Crypto","not_supported","supported",
       {},false,"","",{}},
    }
  }},
  {"blackhawknetwork", {
    "Blackhawknetwork", "Blackhawknetwork connector", "payment_gateway", "alpha",
    {},
    {}
  }},
  {"bluesnap", {
    "BlueSnap", "BlueSnap is a payment platform that helps businesses accept payments from customers in over 200 regions ", "payment_gateway", "live",
    {"payments","refunds","disputes"},
    {
      {"wallet","google_pay","Google Pay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","jcb","diners_club","discover","cartes_bancaires","ru_pay","maestro"}},
    }
  }},
  {"boku", {
    "Boku", "Boku, Inc. is a mobile payments company that allows businesses to collect online payments through both carrier billing and mobile wallets.", "alternative_payment_method", "alpha",
    {},
    {
      {"wallet","dana","DANA","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","gcash","GCash","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","go_pay","GoPay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","kakao_pay","KakaoPay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","momo","MoMo","not_supported","supported",
       {},false,"","",{}},
    }
  }},
  {"braintree", {
    "Braintree", "Braintree, a PayPal service, offers a full-stack payment platform that simplifies accepting payments in your app or website, supporting various payment methods including credit cards and PayPal.", "payment_gateway", "live",
    {"payments","refunds"},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"american_express","discover","jcb","union_pay","mastercard","visa"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"american_express","discover","jcb","union_pay","mastercard","visa"}},
      {"wallet","google_pay","Google Pay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","paypal","PayPal","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
    }
  }},
  {"breadpay", {
    "Breadpay", "Breadpay connector", "payment_gateway", "alpha",
    {},
    {
      {"pay_later","breadpay","Breadpay","not_supported","supported",
       {},false,"","",{}},
    }
  }},
  {"calida", {
    "Calida", "Calida Financial is a licensed e-money institution based in Malta and they provide customized financial infrastructure and payment solutions across the EU and EEA. As part of The Payments Group, it focuses on embedded finance, prepaid services, and next-generation digital payment products.", "alternative_payment_method", "live",
    {"payments"},
    {
      {"wallet","bluecode","Bluecode","not_supported","not_supported",
       {},false,"","",{}},
    }
  }},
  {"cashtocode", {
    "CashToCode", "CashToCode is a payment solution that enables users to convert cash into digital vouchers for online transactions", "payment_gateway", "live",
    {"payments"},
    {
      {"reward","classic","Classic Reward","not_supported","not_supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"reward","evoucher","Evoucher","not_supported","not_supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
    }
  }},
  {"celero", {
    "Celero", "Celero is your trusted provider for payment processing technology and solutions, with a commitment to helping small to mid-sized businesses thrive", "payment_gateway", "alpha",
    {},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"american_express","discover","diners_club","jcb","mastercard","visa"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"american_express","discover","diners_club","jcb","mastercard","visa"}},
    }
  }},
  {"chargebee", {
    "Chargebee", "Chargebee is a Revenue Growth Management (RGM) platform that helps subscription businesses manage subscriptions, billing, revenue recognition, collections, and customer retention, essentially streamlining the entire subscription lifecycle.", "revenue_growth_management_platform", "alpha",
    {"payments"},
    {}
  }},
  {"checkbook", {
    "Checkbook", "Checkbook is a payment platform that allows users to send and receive digital checks.", "payment_gateway", "beta",
    {"payments"},
    {
      {"bank_transfer","ach","ACH Direct Debit","not_supported","not_supported",
       {},false,"","",{}},
    }
  }},
  {"checkout", {
    "Checkout", "Checkout.com is a British multinational financial technology company that processes payments for other companies.", "payment_gateway", "live",
    {"payments","refunds","disputes"},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic","manual_multiple"},true,"supported","supported",{"american_express","cartes_bancaires","diners_club","discover","jcb","mastercard","visa","union_pay"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic","manual_multiple"},true,"supported","supported",{"american_express","cartes_bancaires","diners_club","discover","jcb","mastercard","visa","union_pay"}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual","sequential_automatic","manual_multiple"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","not_supported","supported",
       {"automatic","manual","sequential_automatic","manual_multiple"},false,"","",{}},
      {"network_token","network_token","Network Token","not_supported","supported",
       {"automatic","manual","sequential_automatic","manual_multiple"},false,"","",{}},
    }
  }},
  {"citigate", {
    "Citigate", "Citigate connector", "payment_gateway", "live",
    {},
    {}
  }},
  {"coinbase", {
    "Coinbase", "Coinbase is a place for people and businesses to buy, sell, and manage crypto.", "payment_gateway", "beta",
    {"payments","refunds"},
    {
      {"crypto","crypto_currency","Crypto","not_supported","not_supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
    }
  }},
  {"coingate", {
    "Coingate", "CoinGate's online payment solution makes it easy for businesses to accept Bitcoin, Ethereum, stablecoins and other cryptocurrencies for payments on any website.", "alternative_payment_method", "sandbox",
    {"payments"},
    {}
  }},
  {"cryptopay", {
    "Cryptopay", "Simple and secure solution to buy and manage crypto. Make quick international transfers, spend your BTC, ETH and other crypto assets.", "payment_gateway", "live",
    {"payments"},
    {
      {"crypto","crypto_currency","Crypto","not_supported","not_supported",
       {},false,"","",{}},
    }
  }},
  {"cybersource", {
    "Cybersource", "Cybersource provides payments, fraud protection, integrations, and support to help businesses grow with a digital-first approach.", "payment_gateway", "live",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","jcb","diners_club","discover","visa","cartes_bancaires","union_pay","maestro"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","jcb","diners_club","discover","visa","cartes_bancaires","union_pay","maestro"}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","paze","Paze","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","samsung_pay","Samsung Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
    }
  }},
  {"cybersourcedecisionmanager", {
    "Cybersourcedecisionmanager", "Cybersourcedecisionmanager connector", "payment_gateway", "live",
    {},
    {}
  }},
  {"d24", {
    "D24", "Directa24 WebPay — Transbank's Chilean redirect payment method. The customer is redirected to Transbank's own page to enter card details, so no card data passes through Hyperswitch. Chile only, CLP and USD.", "payment_gateway", "beta",
    {},
    {
      {"card_redirect","card_redirect","Card Redirect","not_supported","supported",
       {},false,"","",{}},
    }
  }},
  {"datatrans", {
    "Datatrans", "Datatrans is a payment gateway that facilitates the processing of payments, including hosting smart payment forms and correctly routing payment information.", "payment_gateway", "live",
    {},
    {}
  }},
  {"deutschebank", {
    "Deutsche Bank", "Deutsche Bank is a German multinational investment bank and financial services company ", "bank_acquirer", "sandbox",
    {},
    {
      {"bank_debit","sepa","SEPA Direct Debit","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","not_supported",{"visa","mastercard"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","not_supported",{"visa","mastercard"}},
    }
  }},
  {"digitalvirgo", {
    "Digital Virgo", "Digital Virgo is an alternative payment provider specializing in carrier billing and mobile payments ", "alternative_payment_method", "alpha",
    {},
    {
      {"mobile_payment","direct_carrier_billing","Direct Carrier Billing","not_supported","supported",
       {},false,"","",{}},
    }
  }},
  {"dlocal", {
    "DLOCAL", "Dlocal is a cross-border payment processor enabling businesses to accept and send payments in emerging markets worldwide.", "payment_gateway", "sandbox",
    {},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","discover","jcb","diners_club","union_pay","interac","cartes_bancaires"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","discover","jcb","diners_club","union_pay","interac","cartes_bancaires"}},
      {"voucher","oxxo","OXXO","not_supported","supported",
       {"automatic"},false,"","",{}},
    }
  }},
  {"dwolla", {
    "Dwolla", "Dwolla is a multinational financial technology company offering financial services and software as a service (SaaS)", "payment_gateway", "sandbox",
    {"payments","refunds"},
    {
      {"bank_debit","ach","ACH Direct Debit","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
    }
  }},
  {"ebanx", {
    "Ebanx", "EBANX payout connector for cross-border disbursements and local currency payouts across Latin America, Africa, and emerging markets", "payout_processor", "sandbox",
    {},
    {}
  }},
  {"ecommpay", {
    "Ecommpay", "Ecommpay is a European payment service provider offering card, wallet and alternative payment processing through its Gate API.", "payment_gateway", "beta",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual"},true,"supported","supported",{"visa","mastercard","american_express","discover","jcb","diners_club","union_pay"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual"},true,"supported","supported",{"visa","mastercard","american_express","discover","jcb","diners_club","union_pay"}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual"},false,"","",{}},
    }
  }},
  {"elavon", {
    "Elavon", "Elavon, a wholly owned subsidiary of U.S. Bank, has been a global leader in payment processing for more than 30 years.", "payment_gateway", "sandbox",
    {},
    {}
  }},
  {"elavon_pg", {
    "Elavon Payment Gateway", "Elavon Payment Gateway (EPG) is Elavon's JSON REST card acquiring gateway. This is a different product from the `elavon` connector, which is Elavon Converge (an XML API).", "payment_gateway", "alpha",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual"},true,"supported","supported",{"visa","mastercard","american_express","jcb","diners_club","discover","union_pay"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual"},true,"supported","supported",{}},
    }
  }},
  {"envoy", {
    "Envoy", "Envoy connector", "payment_gateway", "live",
    {},
    {}
  }},
  {"etisalat", {
    "Etisalat", "Etisalat connector", "payment_gateway", "live",
    {},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{"visa","mastercard"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{"visa","mastercard"}},
    }
  }},
  {"facilitapay", {
    "Facilitapay", "FacilitaPay, the best and most reliable payment provider for your international business.", "payment_gateway", "sandbox",
    {"payments"},
    {
      {"bank_transfer","pix","Pix","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
    }
  }},
  {"finix", {
    "Finix", "Finix is a payments technology provider enabling businesses to accept and send payments online or in person", "payment_gateway", "sandbox",
    {},
    {}
  }},
  {"fiserv", {
    "Fiserv", "Fiserv is a global fintech and payments company with solutions for banking, global commerce, merchant acquiring, billing and payments, and point-of-sale ", "payment_gateway", "sandbox",
    {},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","sequential_automatic","manual"},true,"not_supported","supported",{"visa","mastercard","american_express","jcb","discover","union_pay","interac"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","sequential_automatic","manual"},true,"not_supported","supported",{"visa","mastercard","american_express","jcb","discover","union_pay","interac"}},
      {"wallet","google_pay","Google Pay","not_supported","supported",
       {"automatic","sequential_automatic","manual"},false,"","",{}},
      {"wallet","paypal","PayPal","not_supported","supported",
       {"automatic","sequential_automatic","manual"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","not_supported","supported",
       {"automatic","sequential_automatic","manual"},false,"","",{}},
    }
  }},
  {"fiservcommercehub", {
    "Fiservcommercehub", "Fiservcommercehub is a developer-friendly, flexible communication standards, unified APIs, and pre-certified integrations, Commerce Hub reduces development time and accelerates speed to market.", "payment_gateway", "live",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual"},true,"supported","supported",{"mastercard","visa","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay","maestro"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual"},true,"supported","supported",{"mastercard","visa","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay","maestro"}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual"},false,"","",{}},
    }
  }},
  {"fiservemea", {
    "Fiservemea", "Fiserv powers over 6+ million merchants and 10,000+ financial institutions enabling them to accept billions of payments a year.", "bank_acquirer", "sandbox",
    {},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"mastercard","visa","american_express","discover","jcb","union_pay","diners_club","interac","cartes_bancaires"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"mastercard","visa","american_express","discover","jcb","union_pay","diners_club","interac","cartes_bancaires"}},
    }
  }},
  {"fiuu", {
    "Fiuu", "Fiuu, formerly known as Razer Merchant Services, is a leading online payment gateway in Southeast Asia, offering secure and seamless payment solutions for businesses of all sizes, including credit and debit cards, e-wallets, and bank transfers.", "payment_gateway", "live",
    {"payments","refunds"},
    {}
  }},
  {"flexiti", {
    "Flexiti", "Flexiti connector", "payment_gateway", "alpha",
    {},
    {
      {"pay_later","klarna","Klarna","not_supported","not_supported",
       {},false,"","",{}},
    }
  }},
  {"forte", {
    "Forte", "CSG Forte offers a unified payments platform, enabling businesses to securely process credit cards, debit cards, ACH/eCheck transactions, and more, with advanced fraud prevention and seamless integration.", "payment_gateway", "sandbox",
    {},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"american_express","discover","diners_club","jcb","mastercard","visa"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"american_express","discover","diners_club","jcb","mastercard","visa"}},
    }
  }},
  {"getnet", {
    "Getnet", "Getnet is a high-tech global payment platform that helps businesses accept payments securely while providing the best frictionless experience for customers everywhere.", "payment_gateway", "alpha",
    {"payments","refunds"},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {},true,"not_supported","supported",{"mastercard","visa","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay","ru_pay","maestro"}},
    }
  }},
  {"gigadat", {
    "Gigadat", "Gigadat is a financial services product that offers a single API for payment integration. It provides Canadian businesses with a secure payment gateway and various pay-in and pay-out solutions, including Interac e-Transfer", "payment_gateway", "live",
    {},
    {
      {"bank_redirect","interac","Interac","not_supported","supported",
       {},false,"","",{}},
    }
  }},
  {"givepayments", {
    "Givepayments", "GivePayments connects providers, merchants, and customers through a fully integrated ecosystem of payment tools and services with built-in chargeback prevention, automated underwriting, and PCI DSS 4.0-level security.", "payment_gateway", "alpha",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic"},true,"not_supported","supported",{"visa","mastercard","american_express","jcb","diners_club","discover","union_pay"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic"},true,"not_supported","supported",{}},
    }
  }},
  {"globalpay", {
    "Globalpay", "Global Payments is an American multinational financial technology company that provides payment technology and services to merchants, issuers and consumers.", "payment_gateway", "sandbox",
    {"payments"},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"visa","mastercard","american_express","diners_club","discover","interac","jcb","cartes_bancaires","union_pay"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"visa","mastercard","american_express","diners_club","discover","interac","jcb","cartes_bancaires","union_pay"}},
      {"bank_redirect","ideal","iDEAL","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_redirect","giropay","Giropay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_redirect","sofort","Sofort","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_redirect","eps","EPS","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","paypal","PayPal","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
    }
  }},
  {"globalpayments_heartland", {
    "Global Payments Heartland", "Global Payments' Heartland Portico gateway (Hps.Exchange.PosGateway), a SOAP card-acquiring platform used by merchants across the United States", "payment_gateway", "beta",
    {},
    {}
  }},
  {"globalpayments_realex", {
    "Global Payments RealEx", "Global Payments RealEx is the legacy Global Payments XML (HPP/Remote) gateway, offering card authorisation, settlement, void, rebate and transaction query over the `epage-remote.cgi` endpoint.", "payment_gateway", "beta",
    {},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{"visa","mastercard","american_express","jcb","diners_club","discover","union_pay"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{}},
    }
  }},
  {"globepay", {
    "Globepay", "GlobePay Limited is a professional cross-border payment solution provider (WeChat Pay & Alipay) in the UK", "payment_gateway", "sandbox",
    {},
    {
      {"wallet","ali_pay","Alipay","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"wallet","we_chat_pay","WeChat Pay","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
    }
  }},
  {"gocardless", {
    "GoCardless", "GoCardless is a fintech company that specialises in bank payments including recurring payments.", "payment_gateway", "sandbox",
    {"payments","refunds","mandates"},
    {
      {"bank_debit","ach","ACH Direct Debit","supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_debit","becs","BECS Direct Debit","supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_debit","sepa","SEPA Direct Debit","supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
    }
  }},
  {"gotyme_sanlam", {
    "GotymeSanlam", "GotymeSanlam connector", "payment_gateway", "live",
    {},
    {
      {"bank_debit","eft_debit_order","EFT Debit Order","not_supported","not_supported",
       {"automatic"},false,"","",{}},
    }
  }},
  {"gpayments", {
    "GPayments", "GPayments authentication connector for 3D Secure MPI/ACS services supporting Visa Secure, Mastercard SecureCode, and global card authentication standards", "authentication_provider", "alpha",
    {},
    {}
  }},
  {"helcim", {
    "Helcim", "Helcim is a payment processing company that offers transparent, affordable merchant services for businesses of all sizes", "payment_gateway", "sandbox",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"visa","mastercard","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"visa","mastercard","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay"}},
    }
  }},
  {"hipay", {
    "Hipay", "HiPay is an independent global payment service provider that is based in France.", "payment_gateway", "sandbox",
    {},
    {}
  }},
  {"hyperpg", {
    "Hyperpg", "Hyperpg is your trusted payment gateway solution. Seamlessly manage transactions, enhance security, and empower your business to thrive in the digital age.", "payment_gateway", "live",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic"},true,"supported","supported",{"mastercard","visa","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay","maestro"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic"},true,"supported","supported",{"mastercard","visa","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay","maestro"}},
    }
  }},
  {"iatapay", {
    "Iatapay", "IATA Pay is a payment method for travellers to pay for air tickets purchased online by directly debiting their bank account.", "payment_gateway", "live",
    {"payments","refunds"},
    {
      {"upi","upi_collect","UPI Collect","not_supported","supported",
       {"automatic"},false,"","",{}},
      {"upi","upi_intent","UPI Intent","not_supported","supported",
       {"automatic"},false,"","",{}},
      {"bank_redirect","ideal","iDEAL","not_supported","supported",
       {"automatic"},false,"","",{}},
      {"bank_redirect","local_bank_redirect","Local Bank Redirect","not_supported","supported",
       {"automatic"},false,"","",{}},
      {"real_time_payment","duit_now","DuitNow","not_supported","supported",
       {"automatic"},false,"","",{}},
      {"real_time_payment","fps","FPS","not_supported","supported",
       {"automatic"},false,"","",{}},
      {"real_time_payment","prompt_pay","PromptPay","not_supported","supported",
       {"automatic"},false,"","",{}},
      {"real_time_payment","viet_qr","VietQR","not_supported","supported",
       {"automatic"},false,"","",{}},
    }
  }},
  {"ilixium", {
    "Ilixium", "GivePayments connects providers, merchants, and customers through a fully integrated ecosystem of payment tools and services with built-in chargeback prevention, automated underwriting, and PCI DSS 4.0-level security.", "payment_gateway", "alpha",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic"},true,"not_supported","supported",{"visa","mastercard","american_express","jcb","diners_club","discover","union_pay"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic"},true,"not_supported","supported",{}},
    }
  }},
  {"imerchantsolutions", {
    "iMerchant Solutions", "iMerchant Solutions is a modern payment processing platform that empowers businesses to accept payments globally with fast and low-friction onboarding.", "payment_gateway", "beta",
    {"payments","refunds"},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","manual_multiple"},true,"not_supported","supported",{"mastercard","visa","american_express","jcb","discover","union_pay"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","manual_multiple"},true,"not_supported","supported",{}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual","manual_multiple"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {},false,"","",{}},
    }
  }},
  {"inespay", {
    "Inespay", "INESPAY is a payment method system that allows online shops to receive money in their bank accounts through a SEPA bank transfer ", "bank_acquirer", "sandbox",
    {"payments","refunds"},
    {
      {"bank_debit","sepa","SEPA Direct Debit","not_supported","supported",
       {"automatic"},false,"","",{}},
    }
  }},
  {"itaubank", {
    "Itaubank", "Itau Bank is a leading Brazilian financial institution offering a wide range of banking services, including retail banking, loans, and investment solutions.", "payment_gateway", "sandbox",
    {},
    {
      {"bank_transfer","pix","Pix","not_supported","supported",
       {},false,"","",{}},
    }
  }},
  {"jpmorgan", {
    "Jpmorgan", "J.P. Morgan is a global financial services firm and investment bank, offering banking, asset management, and payment processing solutions", "bank_acquirer", "beta",
    {},
    {
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{"american_express","diners_club","discover","jcb","mastercard","union_pay","visa"}},
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{"american_express","diners_club","discover","jcb","mastercard","union_pay","visa"}},
    }
  }},
  {"jpmorgan_orbital", {
    "JP Morgan Orbital", "JP Morgan Orbital (Chase Paymentech Orbital Gateway) is JPMorgan's card acquiring gateway for card-present and card-not-present payments. This is the Orbital Gateway JSON API v4, a different product from the `jpmorgan` connector, which is the JPMorgan Payments API v2.", "payment_gateway", "alpha",
    {},
    {
      {"card","credit","Credit Card","not_supported","not_supported",
       {"automatic"},true,"supported","supported",{"visa","mastercard","american_express","jcb","diners_club","discover","union_pay","maestro"}},
      {"card","debit","Debit Card","not_supported","not_supported",
       {"automatic"},true,"supported","supported",{}},
    }
  }},
  {"juspaythreedsserver", {
    "Juspay 3Ds Server", "Juspay 3DS Server provider for comprehensive 3-Domain Secure authentication, cardholder verification, and fraud prevention across card networks", "authentication_provider", "alpha",
    {},
    {}
  }},
  {"klarna", {
    "Klarna", "Klarna provides payment processing services for the e-commerce industry, managing store claims and customer payments.", "payment_gateway", "live",
    {},
    {
      {"pay_later","klarna","Klarna","not_supported","supported",
       {},false,"","",{}},
    }
  }},
  {"loonio", {
    "Loonio", "Loonio is a payment processing platform that provides APIs for deposits and payouts via methods like Interac, PIX, EFT, and credit cards, with webhook support and transaction sync for real-time and manual status tracking.", "payment_gateway", "live",
    {"payments","payouts"},
    {
      {"bank_redirect","interac","Interac","not_supported","supported",
       {},false,"","",{}},
    }
  }},
  {"merchante", {
    "Merchante", "Merchante connector", "payment_gateway", "live",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{"visa","mastercard","american_express","discover"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{"visa","mastercard","american_express","discover"}},
    }
  }},
  {"mifinity", {
    "MIFINITY", "Mifinity is a payment gateway empowering you to pay online, receive funds, and send money globally, the MiFinity eWallet supports super-low fees, offering infinite possibilities to do more of the things you love.", "payment_gateway", "live",
    {},
    {
      {"wallet","mifinity","MiFinity","not_supported","not_supported",
       {},false,"","",{}},
    }
  }},
  {"moneris", {
    "Moneris", "Moneris is Canada's top payment solutions provider, empowering businesses with innovative mobile, online & in-store solutions.", "payment_gateway", "sandbox",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual"},true,"supported","supported",{"mastercard","visa","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual"},true,"supported","supported",{"mastercard","visa","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay"}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual"},false,"","",{}},
    }
  }},
  {"multisafepay", {
    "Multisafepay", "MultiSafePay is a payment gateway and PSP enabling secure online transactions", "payment_gateway", "sandbox",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","sequential_automatic"},true,"supported","not_supported",{"american_express","cartes_bancaires","maestro","mastercard","visa"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","sequential_automatic"},true,"supported","not_supported",{"american_express","cartes_bancaires","maestro","mastercard","visa"}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"wallet","paypal","PayPal","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"wallet","ali_pay","Alipay","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"wallet","we_chat_pay","WeChat Pay","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_redirect","giropay","Giropay","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_redirect","trustly","Trustly","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_redirect","ideal","iDEAL","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"pay_later","klarna","Klarna","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_redirect","eps","EPS","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_redirect","sofort","Sofort","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"wallet","mb_way","MB WAY","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
    }
  }},
  {"netcetera", {
    "Netcetera", "Netcetera authentication provider for comprehensive 3D Secure solutions including certified ACS, Directory Server, and multi-protocol EMV 3DS supports", "authentication_provider", "sandbox",
    {},
    {}
  }},
  {"nexinets", {
    "Nexinets", "Nexi and Nets join forces to create The European PayTech leader, a strategic combination to offer future-proof innovative payment solutions.", "payment_gateway", "sandbox",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","diners_club","discover","interac","jcb","cartes_bancaires","union_pay"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","diners_club","discover","interac","jcb","cartes_bancaires","union_pay"}},
      {"bank_redirect","ideal","iDEAL","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_redirect","giropay","Giropay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_redirect","sofort","Sofort","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_redirect","eps","EPS","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","paypal","PayPal","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
    }
  }},
  {"nexixpay", {
    "Nexixpay", "Nexixpay is an Italian bank that specialises in payment systems such as Nexi Payments (formerly known as CartaSi).", "payment_gateway", "sandbox",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","jcb"}},
      {"card","debit","Debit Card","supported","supported",
       {},true,"supported","supported",{"visa","mastercard","american_express","jcb"}},
    }
  }},
  {"nmi", {
    "NMI", "NMI is a global leader in embedded payments, powering more than $200 billion in payment volumes every year.", "payment_gateway", "live",
    {"payments","refunds"},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"mastercard","visa","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay","maestro"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"mastercard","visa","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay","maestro"}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {},false,"","",{}},
    }
  }},
  {"nomupay", {
    "Nomupay", "Nomupay payouts connector for disbursements to recipients' bank accounts and alternative payment methods in Southeast Asia and the Pacific Islands", "payout_processor", "sandbox",
    {},
    {}
  }},
  {"noon", {
    "Noon", "Noon is a payment gateway and PSP enabling secure online transactions", "payment_gateway", "live",
    {"payments"},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"american_express","jcb","mastercard","visa"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"american_express","jcb","mastercard","visa"}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","paypal","PayPal","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
    }
  }},
  {"nordea", {
    "Nordea", "Nordea is one of the leading financial services group in the Nordics and the preferred choice for millions across the region.", "payment_gateway", "beta",
    {},
    {
      {"bank_debit","sepa","SEPA Direct Debit","not_supported","not_supported",
       {"automatic","sequential_automatic"},false,"","",{}},
    }
  }},
  {"novalnet", {
    "Novalnet", "Novalnet provides tailored, data-driven payment solutions that maximize acceptance, boost conversions, and deliver seamless customer experiences worldwide.", "payment_gateway", "live",
    {"payments","refunds","disputes","mandates"},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","jcb","diners_club","union_pay","discover","cartes_bancaires","interac"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","jcb","diners_club","union_pay","discover","cartes_bancaires","interac"}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","paypal","PayPal","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_debit","sepa","SEPA Direct Debit","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_debit","sepa_guarenteed_debit","SEPA Guaranteed Direct Debit","supported","supported",
       {},false,"","",{}},
    }
  }},
  {"nuvei", {
    "Nuvei", "Nuvei is the Canadian fintech company accelerating the business of clients around the world.", "payment_gateway", "live",
    {"payments","disputes"},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","union_pay","interac","jcb","diners_club","discover","cartes_bancaires"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","union_pay","interac","jcb","diners_club","discover","cartes_bancaires"}},
      {"pay_later","klarna","Klarna","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"pay_later","afterpay_clearpay","Afterpay Clearpay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_redirect","ideal","iDEAL","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_redirect","giropay","Giropay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_redirect","sofort","Sofort","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_redirect","eps","EPS","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","paypal","PayPal","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"network_token","network_token","Network Token","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
    }
  }},
  {"opennode", {
    "Opennode", "OpenNode offers accessible way for e-commerce businesses to process bitcoin payments.", "payment_gateway", "beta",
    {"payments"},
    {
      {"crypto","crypto_currency","Crypto","not_supported","not_supported",
       {},false,"","",{}},
    }
  }},
  {"paybox", {
    "Paybox", "Paybox is a payment gateway that enables businesses to process online transactions securely ", "payment_gateway", "sandbox",
    {},
    {
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay"}},
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay"}},
    }
  }},
  {"payconex", {
    "Payconex", "Payconex connector", "payment_gateway", "live",
    {},
    {}
  }},
  {"paydotcom", {
    "Pay.com", "Pay.com is a payment gateway offering card acquiring over its REST API v1.", "payment_gateway", "beta",
    {},
    {}
  }},
  {"payhound", {
    "Payhound", "Payhound is a hosted cryptocurrency payment gateway that lets merchants accept crypto payments through a redirect based invoice checkout.", "payment_gateway", "beta",
    {"payments"},
    {
      {"crypto","crypto_currency","Crypto","not_supported","not_supported",
       {},false,"","",{}},
    }
  }},
  {"payjustnow", {
    "Payjustnow", "PayJustNow is a South African payment connector that enables customers to split online purchases into three interest-free monthly installments.", "payment_gateway", "sandbox",
    {"payments"},
    {
      {"pay_later","payjustnow","Payjustnow","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
    }
  }},
  {"payjustnowinstore", {
    "PayJustNow In-Store", "PayJustNow provides a BNPL solution for online and in-store payments, enabling customers to pay in three interest-free installments while merchants get paid upfront.", "payment_gateway", "sandbox",
    {"payments"},
    {
      {"pay_later","payjustnow","Payjustnow","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
    }
  }},
  {"payload", {
    "Payload", "Payload is an embedded finance solution for modern platforms and businesses, automating inbound and outbound payments with an industry-leading platform and driving innovation into the future.", "payment_gateway", "sandbox",
    {"disputes","payments","refunds"},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"american_express","discover","mastercard","visa","diners_club"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"american_express","discover","mastercard","visa","diners_club"}},
      {"bank_debit","ach","ACH Direct Debit","supported","supported",
       {},false,"","",{}},
    }
  }},
  {"payme", {
    "Payme", "Payme is a payment gateway enabling secure online transactions", "payment_gateway", "live",
    {"payments","refunds","disputes"},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"american_express","diners_club","discover","jcb","mastercard","visa"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"american_express","diners_club","discover","jcb","mastercard","visa"}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
    }
  }},
  {"paynearme", {
    "PayNearMe", "PayNearMe is a US payments platform for bill pay and iGaming, offering card, ACH and cash payments.", "payment_gateway", "alpha",
    {},
    {}
  }},
  {"payone", {
    "Payone", "Payone payout connector for European market disbursements and automated fund distribution with comprehensive compliance support", "payout_processor", "sandbox",
    {},
    {}
  }},
  {"paypal", {
    "Paypal", "PayPal is a global online payment system that enables individuals and businesses to send and receive money electronically.", "payment_gateway", "live",
    {"payments","refunds","disputes"},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"mastercard","visa","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"mastercard","visa","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay"}},
      {"wallet","paypal","PayPal","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_redirect","eps","EPS","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_redirect","giropay","Giropay","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_redirect","ideal","iDEAL","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_redirect","sofort","Sofort","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
    }
  }},
  {"paysafe", {
    "Paysafe", "Paysafe gives ambitious businesses a launchpad with safe, secure online payment solutions, and gives consumers the ability to turn their transactions into meaningful experiences.", "payment_gateway", "sandbox",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{"mastercard","visa","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{"mastercard","visa","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay"}},
      {"wallet","apple_pay","Apple Pay","not_supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"wallet","skrill","Skrill","not_supported","supported",
       {"automatic"},false,"","",{}},
      {"wallet","neteller","Neteller","not_supported","supported",
       {"automatic"},false,"","",{}},
      {"bank_redirect","interac","Interac","not_supported","supported",
       {"automatic"},false,"","",{}},
      {"gift_card","pay_safe_card","PaySafeCard","not_supported","supported",
       {"automatic"},false,"","",{}},
    }
  }},
  {"paystack", {
    "Paystack", "Paystack is a Nigerian financial technology company that provides online and offline payment solutions to businesses across Africa.", "payment_gateway", "sandbox",
    {"payments","refunds"},
    {
      {"bank_redirect","eft","EFT","not_supported","supported",
       {},false,"","",{}},
    }
  }},
  {"paytm", {
    "PAYTM", "Paytm is an Indian multinational fintech company specializing in digital payments and financial services. Initially known for its mobile wallet, it has expanded to include a payment bank, e-commerce, ticketing, and wealth management services.", "payment_gateway", "alpha",
    {},
    {
      {"upi","upi_collect","UPI Collect","not_supported","not_supported",
       {"automatic"},false,"","",{}},
      {"upi","upi_intent","UPI Intent","not_supported","not_supported",
       {},false,"","",{}},
    }
  }},
  {"payu", {
    "Payu", "PayU is a global fintech company providing online payment solutions, including card processing, UPI, wallets, and BNPL services across multiple markets ", "payment_gateway", "sandbox",
    {},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","not_supported",{"visa","mastercard","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","not_supported",{"visa","mastercard","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay"}},
      {"wallet","google_pay","Google Pay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
    }
  }},
  {"peachpayments", {
    "Peach Payments", "The secure African payment gateway with easy integrations, 365-day support, and advanced orchestration.", "payment_gateway", "live",
    {"payments","refunds"},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{"visa","mastercard","american_express"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{"visa","mastercard","american_express"}},
      {"network_token","network_token","Network Token","not_supported","supported",
       {},false,"","",{}},
    }
  }},
  {"phonepe", {
    "PHONEPE", "PhonePe is a digital payments and financial services platform built on the UPI system. It allows users to make instant payments, recharge mobiles, pay bills, and access financial services like investments and insurance.", "payment_gateway", "alpha",
    {},
    {
      {"upi","upi_collect","UPI Collect","not_supported","not_supported",
       {"automatic"},false,"","",{}},
      {"upi","upi_intent","UPI Intent","not_supported","not_supported",
       {},false,"","",{}},
    }
  }},
  {"placetopay", {
    "PlacetoPay", "PlacetoPay is a Latin American financial technology company's online payment platform, offering various payment methods and integrations for businesses", "payment_gateway", "sandbox",
    {},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","sequential_automatic"},true,"not_supported","supported",{"american_express","diners_club","mastercard","visa"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","sequential_automatic"},true,"not_supported","supported",{"american_express","diners_club","mastercard","visa"}},
    }
  }},
  {"plaid", {
    "Plaid", "Plaid is a data network that helps millions connect their financial accounts to apps like Venmo, SoFi, and Betterment. It powers tools used by Fortune 500 companies, major banks, and leading fintechs to enable easier, smarter financial lives.", "authentication_provider", "beta",
    {},
    {
      {"open_banking","open_banking_pis","Open Banking PIS","not_supported","not_supported",
       {},false,"","",{}},
    }
  }},
  {"powertranz", {
    "Powertranz", "Powertranz is a leading payment gateway serving the Caribbean and parts of Central America ", "payment_gateway", "alpha",
    {},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express"}},
    }
  }},
  {"prophetpay", {
    "Prophetpay", "GlobePay Limited is a professional cross-border payment solution provider (WeChat Pay & Alipay) in the UK", "payment_gateway", "alpha",
    {},
    {
      {"card_redirect","card_redirect","Card Redirect","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
    }
  }},
  {"rapyd", {
    "Rapyd", "Rapyd is a fintech company that enables businesses to collect payments in local currencies across the globe ", "payment_gateway", "sandbox",
    {"payments","refunds","disputes"},
    {
      {"wallet","apple_pay","Apple Pay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","google_pay","Google Pay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"american_express","visa","jcb","diners_club","union_pay","mastercard","discover"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"american_express","visa","jcb","diners_club","union_pay","mastercard","discover"}},
    }
  }},
  {"razorpay", {
    "RAZORPAY", "Razorpay helps you accept online payments from customers across Desktop, Mobile web, Android & iOS. Additionally by using Razorpay Payment Links, you can collect payments across multiple channels like SMS, Email, Whatsapp, Chatbots & Messenger.", "payment_gateway", "sandbox",
    {"payments","refunds"},
    {
      {"upi","upi_collect","UPI Collect","not_supported","supported",
       {},false,"","",{}},
    }
  }},
  {"redsys", {
    "Redsys", "Redsys is a Spanish payment gateway offering secure and innovative payment solutions for merchants and banks", "payment_gateway", "live",
    {},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual"},true,"supported","not_supported",{"visa","mastercard","american_express","discover","jcb","diners_club","union_pay"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual"},true,"supported","not_supported",{"visa","mastercard","american_express","discover","jcb","diners_club","union_pay"}},
    }
  }},
  {"revolv3", {
    "Revolv3", "Revolv3 is a payment platform that enables secure card processing and alternative payment solutions for merchants through a unified API.", "payment_gateway", "beta",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","discover","jcb","diners_club","union_pay"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","discover","jcb","diners_club","union_pay"}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
    }
  }},
  {"riskified", {
    "Riskified", "Riskified fraud and risk management provider with guaranteed real-time decisions and machine learning-powered ecommerce fraud prevention", "fraud_and_risk_management_provider", "sandbox",
    {},
    {}
  }},
  {"saferpay", {
    "Saferpay", "Saferpay is the payment gateway of SIX Payment Services (Worldline), widely used by merchants in Switzerland, Austria, Germany and Luxembourg", "payment_gateway", "beta",
    {},
    {}
  }},
  {"santander", {
    "Santander", "Santander is a leading private bank in Brazil, offering a wide range of financial services across retail and corporate segments. It is part of the global Santander Group, one of Europe’s largest financial institutions.", "bank_acquirer", "alpha",
    {},
    {
      {"bank_transfer","pix_qr","Pix QR","not_supported","supported",
       {"automatic"},false,"","",{}},
      {"bank_transfer","pix_automatico_qr","Pix Automático QR","supported","supported",
       {"automatic"},false,"","",{}},
      {"bank_transfer","pix_automatico_push","Pix Automático Push","supported","supported",
       {"automatic"},false,"","",{}},
      {"voucher","boleto","Boleto Bancário","not_supported","not_supported",
       {},false,"","",{}},
    }
  }},
  {"shift4", {
    "Shift4", "Shift4 Payments, Inc. is an American payment processing company based in Allentown, Pennsylvania. ", "payment_gateway", "live",
    {"payments","refunds"},
    {
      {"bank_redirect","eps","EPS","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_redirect","giropay","Giropay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_redirect","ideal","iDEAL","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_redirect","sofort","Sofort","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard"}},
    }
  }},
  {"signifyd", {
    "Signifyd", "Signifyd fraud and risk management provider with AI-driven commerce protection platform for maximizing conversions and eliminating fraud risk with guaranteed fraud liability coverage", "fraud_and_risk_management_provider", "sandbox",
    {},
    {}
  }},
  {"silverflow", {
    "Silverflow", "Silverflow is a global payment processor that provides secure and reliable payment processing services with support for multiple capture methods and 3DS authentication.", "payment_gateway", "alpha",
    {"payments","refunds"},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{"visa","mastercard","american_express","discover"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{"visa","mastercard","american_express","discover"}},
    }
  }},
  {"square", {
    "Square", "Square is the largest business technology platform serving all kinds of businesses.", "payment_gateway", "alpha",
    {"payments","refunds"},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"visa","mastercard","american_express","discover","union_pay","interac","jcb"}},
      {"card","debit","Debit Card","not_supported","supported",
       {},true,"not_supported","supported",{"visa","mastercard","american_express","discover","union_pay","interac","jcb"}},
    }
  }},
  {"stax", {
    "Stax", "Stax is a payment processing platform that helps businesses accept payments and manage their payment ecosystem ", "payment_gateway", "sandbox",
    {"payments","refunds"},
    {
      {"bank_debit","ach","ACH Direct Debit","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"visa","mastercard","american_express","discover"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"visa","mastercard","american_express","discover"}},
    }
  }},
  {"stripe", {
    "Stripe", "Stripe is a payment processing platform that provides businesses with tools and APIs to accept payments online and manage their financial infrastructure", "payment_gateway", "live",
    {"payments","refunds","disputes"},
    {}
  }},
  {"stripebilling", {
    "Stripebilling", "Stripe Billing manages subscriptions, recurring payments, and invoicing. It supports trials, usage-based billing, coupons, and automated retries.", "revenue_growth_management_platform", "beta",
    {"payments"},
    {}
  }},
  {"taxjar", {
    "Taxjar", "TaxJar is a cloud-based platform that automates sales tax calculations, reporting, and filing for businesses across multiple channels", "tax_calculation_provider", "sandbox",
    {},
    {}
  }},
  {"tesouro", {
    "Tesouro", "Tesouro is a Miami-based fintech company that provides a cloud-native payment gateway platform, offering APIs and streamlined data inflows to connect software companies, banks, and payment facilitators for seamless payment processing", "payment_gateway", "sandbox",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{"mastercard","visa","american_express","discover","diners_club","jcb","maestro","union_pay"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{"mastercard","visa","american_express","discover","diners_club","jcb","maestro","union_pay"}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {"automatic","manual"},false,"","",{}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual"},false,"","",{}},
    }
  }},
  {"threedsecureio", {
    "3dsecure.io", "3DSecure.io is a service that facilitates 3-D Secure verifications for online credit and debit card transactions through a simple JSON API, enhancing payment security for merchants.docs.3dsecure.io3dsecure.io", "authentication_provider", "sandbox",
    {},
    {}
  }},
  {"thunes", {
    "Thunes", "Thunes Payouts is a global payment solution that enables businesses to send instant, secure, and cost-effective cross-border payments to bank accounts, mobile wallets, and cards in over 130 countries using a single API", "payout_processor", "sandbox",
    {},
    {}
  }},
  {"tokenex", {
    "Tokenex", "Tokenex connector", "payment_gateway", "alpha",
    {},
    {}
  }},
  {"tokenio", {
    "Token.io", "Token.io is a financial technology company that provides Open Banking and real-time payment solutions across Europe. They enable secure bank-to-bank transfers using various payment rails including SEPA, Faster Payments, and other regional payment systems.", "payment_gateway", "sandbox",
    {"payments"},
    {}
  }},
  {"truelayer", {
    "Truelayer", "Truelayer connector", "payment_gateway", "live",
    {},
    {
      {"bank_redirect","open_banking","Open Banking","not_supported","supported",
       {"automatic","manual"},false,"","",{}},
    }
  }},
  {"trustly", {
    "Trustly", "Trustly connector", "payment_gateway", "live",
    {},
    {
      {"bank_redirect","trustly","Trustly","not_supported","supported",
       {"automatic","manual"},false,"","",{}},
    }
  }},
  {"trustpay", {
    "Trustpay", "TrustPay offers cross-border payment solutions for online businesses, including global card processing, local payment methods, business accounts, and reconciliation tools—all under one roof.", "payment_gateway", "live",
    {"payments","refunds","disputes"},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","sequential_automatic"},true,"not_supported","supported",{"mastercard","visa","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","sequential_automatic"},true,"not_supported","supported",{"mastercard","visa","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay"}},
      {"wallet","google_pay","Google Pay","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_redirect","eps","EPS","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_redirect","giropay","Giropay","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_redirect","ideal","iDEAL","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_redirect","sofort","Sofort","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_redirect","blik","BLIK","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_transfer","sepa_bank_transfer","SEPA Bank Transfer","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_transfer","instant_bank_transfer","Instant Bank Transfer","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_transfer","instant_bank_transfer_finland","Instant Bank Transfer Finland","not_supported","supported",
       {"automatic","sequential_automatic"},false,"","",{}},
      {"bank_transfer","instant_bank_transfer_poland","Instant Bank Transfer Poland","not_supported","supported",
       {},false,"","",{}},
      {"network_token","network_token","Network Token","not_supported","supported",
       {},true,"supported","supported",{"mastercard","visa","interac","american_express","jcb","diners_club","discover","cartes_bancaires","union_pay"}},
    }
  }},
  {"trustpayments", {
    "Trustpayments", "Trustpayments connector", "payment_gateway", "alpha",
    {},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{}},
    }
  }},
  {"tsys", {
    "Tsys", "TSYS, a Global Payments company, is the payment stack for the future, powered by unmatched expertise.", "bank_acquirer", "beta",
    {},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"mastercard","visa","american_express","discover","jcb","union_pay"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"mastercard","visa","american_express","discover","jcb","union_pay"}},
    }
  }},
  {"tsys_transit", {
    "TsysTransit", "TSYS XML (TransIT) is a TSYS gateway integration using the TransIT XML API for card processing.", "payment_gateway", "beta",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","discover","jcb","diners_club","union_pay"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"visa","mastercard","american_express","discover","jcb","diners_club","union_pay"}},
    }
  }},
  {"volt", {
    "VOLT", "Volt is a payment gateway operating in China, specializing in facilitating local bank transfers", "payment_gateway", "live",
    {"payments","refunds"},
    {
      {"bank_redirect","open_banking_uk","Open Banking UK","not_supported","supported",
       {"automatic"},false,"","",{}},
      {"bank_redirect","open_banking","Open Banking","not_supported","supported",
       {},false,"","",{}},
    }
  }},
  {"wellsfargo", {
    "Wells Fargo", "Wells Fargo is a major bank offering retail, commercial, and wealth management services", "bank_acquirer", "beta",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"american_express","discover","mastercard","visa"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"american_express","discover","mastercard","visa"}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_debit","ach","ACH Direct Debit","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
    }
  }},
  {"wise", {
    "Wise", "The Wise connector enables cross-border money transfers by integrating with Wise's API to initiate, track, and manage international payouts efficiently.", "payout_processor", "sandbox",
    {},
    {}
  }},
  {"worldline", {
    "Worldline", "Worldpay is an industry leading payments technology and solutions company with unique capabilities to power omni-commerce across the globe.r", "payment_gateway", "sandbox",
    {"payments"},
    {
      {"bank_redirect","giropay","Giropay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_redirect","ideal","iDEAL","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"american_express","discover","mastercard","visa"}},
      {"card","debit","Debit Card","not_supported","supported",
       {},true,"supported","supported",{"american_express","discover","mastercard","visa"}},
    }
  }},
  {"worldpay", {
    "Worldpay", "Worldpay is a payment gateway and PSP enabling secure online transactions", "payment_gateway", "live",
    {"payments"},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"american_express","cartes_bancaires","diners_club","jcb","maestro","mastercard","visa"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"american_express","cartes_bancaires","diners_club","jcb","maestro","mastercard","visa"}},
      {"wallet","google_pay","Google Pay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
    }
  }},
  {"worldpaymodular", {
    "Worldpaymodular", "Worldpaymodular is a payment gateway and PSP enabling secure online transactions, It utilizes modular Api's of worldpay", "payment_gateway", "beta",
    {"payments","refunds","mandates"},
    {
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
    }
  }},
  {"worldpayraft", {
    "Worldpayraft", "Worldpay Native RAFT connector", "payment_gateway", "alpha",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{"visa","mastercard","american_express","discover","jcb"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual"},true,"not_supported","supported",{"visa","mastercard","american_express","discover","jcb"}},
      {"network_token","network_token","Network Token","supported","supported",
       {"automatic","manual"},false,"","",{}},
    }
  }},
  {"worldpayvantiv", {
    "Worldpay Vantiv", "Worldpay Vantiv, also known as the Worldpay CNP API, is a robust XML-based interface used to process online (card-not-present) transactions such as e-commerce purchases, subscription billing, and digital payments", "payment_gateway", "sandbox",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"american_express","diners_club","jcb","mastercard","visa","discover"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"american_express","diners_club","jcb","mastercard","visa","discover"}},
      {"card","card","Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"american_express","diners_club","jcb","mastercard","visa","discover"}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
    }
  }},
  {"worldpayxml", {
    "Worldpay XML", "Worldpay is a payment gateway and PSP enabling secure online transactions", "payment_gateway", "sandbox",
    {"payments","refunds","payouts"},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"american_express","cartes_bancaires","diners_club","jcb","maestro","mastercard","visa"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"supported","supported",{"american_express","cartes_bancaires","diners_club","jcb","maestro","mastercard","visa"}},
      {"wallet","google_pay","Google Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
    }
  }},
  {"xendit", {
    "Xendit", "Xendit is a financial technology company that provides payment solutions and simplifies the payment process for businesses in Indonesia, the Philippines and Southeast Asia, from SMEs and e-commerce startups to large enterprises.", "payment_gateway", "sandbox",
    {"payments"},
    {}
  }},
  {"zen", {
    "Zen", "Zen Payment Gateway is a secure and scalable payment solution that enables businesses to accept online payments globally with various methods and currencies.", "payment_gateway", "live",
    {"payments","refunds"},
    {
      {"card","credit","Credit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"visa","mastercard","american_express","diners_club","discover","interac","jcb","cartes_bancaires","union_pay"}},
      {"card","debit","Debit Card","not_supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"visa","mastercard","american_express","diners_club","discover","interac","jcb","cartes_bancaires","union_pay"}},
      {"voucher","boleto","Boleto Bancário","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"voucher","efecty","Efecty","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"voucher","pago_efectivo","PagoEfectivo","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"voucher","red_compra","RedCompra","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"voucher","red_pagos","RedPagos","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_transfer","multibanco","Multibanco","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_transfer","pix","Pix","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"bank_transfer","pse","PSE","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","apple_pay","Apple Pay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
      {"wallet","google_pay","Google Pay","not_supported","supported",
       {"automatic","manual","sequential_automatic"},false,"","",{}},
    }
  }},
  {"zift", {
    "Zift", "Zift connector", "payment_gateway", "alpha",
    {},
    {
      {"card","credit","Credit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"american_express","diners_club","jcb","mastercard","visa","discover"}},
      {"card","debit","Debit Card","supported","supported",
       {"automatic","manual","sequential_automatic"},true,"not_supported","supported",{"american_express","diners_club","jcb","mastercard","visa","discover"}},
    }
  }},
  {"zsl", {
    "ZSL", "Zsl is a payment gateway operating in China, specializing in facilitating local bank transfers", "payment_gateway", "live",
    {},
    {
      {"bank_transfer","local_bank_transfer","Local Bank Transfer","not_supported","not_supported",
       {},false,"","",{}},
    }
  }},
};
} // namespace catalog

// CountryAlpha2::from_alpha2_to_alpha3 (common_enums/src/transformers.rs:23)
inline const std::map<std::string_view,std::string_view> kAlpha2ToAlpha3{
  {"AF","AFG"},
  {"AX","ALA"},
  {"AL","ALB"},
  {"DZ","DZA"},
  {"AS","ASM"},
  {"AD","AND"},
  {"AO","AGO"},
  {"AI","AIA"},
  {"AQ","ATA"},
  {"AG","ATG"},
  {"AR","ARG"},
  {"AM","ARM"},
  {"AW","ABW"},
  {"AU","AUS"},
  {"AT","AUT"},
  {"AZ","AZE"},
  {"BS","BHS"},
  {"BH","BHR"},
  {"BD","BGD"},
  {"BB","BRB"},
  {"BY","BLR"},
  {"BE","BEL"},
  {"BZ","BLZ"},
  {"BJ","BEN"},
  {"BM","BMU"},
  {"BT","BTN"},
  {"BO","BOL"},
  {"BQ","BES"},
  {"BA","BIH"},
  {"BW","BWA"},
  {"BV","BVT"},
  {"BR","BRA"},
  {"IO","IOT"},
  {"BN","BRN"},
  {"BG","BGR"},
  {"BF","BFA"},
  {"BI","BDI"},
  {"CV","CPV"},
  {"KH","KHM"},
  {"CM","CMR"},
  {"CA","CAN"},
  {"KY","CYM"},
  {"CF","CAF"},
  {"TD","TCD"},
  {"CL","CHL"},
  {"CN","CHN"},
  {"CX","CXR"},
  {"CC","CCK"},
  {"CO","COL"},
  {"KM","COM"},
  {"CG","COG"},
  {"CD","COD"},
  {"CK","COK"},
  {"CR","CRI"},
  {"CI","CIV"},
  {"HR","HRV"},
  {"CU","CUB"},
  {"CW","CUW"},
  {"CY","CYP"},
  {"CZ","CZE"},
  {"DK","DNK"},
  {"DJ","DJI"},
  {"DM","DMA"},
  {"DO","DOM"},
  {"EC","ECU"},
  {"EG","EGY"},
  {"SV","SLV"},
  {"GQ","GNQ"},
  {"ER","ERI"},
  {"EE","EST"},
  {"ET","ETH"},
  {"FK","FLK"},
  {"FO","FRO"},
  {"FJ","FJI"},
  {"FI","FIN"},
  {"FR","FRA"},
  {"GF","GUF"},
  {"PF","PYF"},
  {"TF","ATF"},
  {"GA","GAB"},
  {"GM","GMB"},
  {"GE","GEO"},
  {"DE","DEU"},
  {"GH","GHA"},
  {"GI","GIB"},
  {"GR","GRC"},
  {"GL","GRL"},
  {"GD","GRD"},
  {"GP","GLP"},
  {"GU","GUM"},
  {"GT","GTM"},
  {"GG","GGY"},
  {"GN","GIN"},
  {"GW","GNB"},
  {"GY","GUY"},
  {"HT","HTI"},
  {"HM","HMD"},
  {"VA","VAT"},
  {"HN","HND"},
  {"HK","HKG"},
  {"HU","HUN"},
  {"IS","ISL"},
  {"IN","IND"},
  {"ID","IDN"},
  {"IR","IRN"},
  {"IQ","IRQ"},
  {"IE","IRL"},
  {"IM","IMN"},
  {"IL","ISR"},
  {"IT","ITA"},
  {"JM","JAM"},
  {"JP","JPN"},
  {"JE","JEY"},
  {"JO","JOR"},
  {"KZ","KAZ"},
  {"KE","KEN"},
  {"KI","KIR"},
  {"KP","PRK"},
  {"KR","KOR"},
  {"KW","KWT"},
  {"KG","KGZ"},
  {"LA","LAO"},
  {"LV","LVA"},
  {"LB","LBN"},
  {"LS","LSO"},
  {"LR","LBR"},
  {"LY","LBY"},
  {"LI","LIE"},
  {"LT","LTU"},
  {"LU","LUX"},
  {"MO","MAC"},
  {"MK","MKD"},
  {"MG","MDG"},
  {"MW","MWI"},
  {"MY","MYS"},
  {"MV","MDV"},
  {"ML","MLI"},
  {"MT","MLT"},
  {"MH","MHL"},
  {"MQ","MTQ"},
  {"MR","MRT"},
  {"MU","MUS"},
  {"YT","MYT"},
  {"MX","MEX"},
  {"FM","FSM"},
  {"MD","MDA"},
  {"MC","MCO"},
  {"MN","MNG"},
  {"ME","MNE"},
  {"MS","MSR"},
  {"MA","MAR"},
  {"MZ","MOZ"},
  {"MM","MMR"},
  {"NA","NAM"},
  {"NR","NRU"},
  {"NP","NPL"},
  {"NL","NLD"},
  {"NC","NCL"},
  {"NZ","NZL"},
  {"NI","NIC"},
  {"NE","NER"},
  {"NG","NGA"},
  {"NU","NIU"},
  {"NF","NFK"},
  {"MP","MNP"},
  {"NO","NOR"},
  {"OM","OMN"},
  {"PK","PAK"},
  {"PW","PLW"},
  {"PS","PSE"},
  {"PA","PAN"},
  {"PG","PNG"},
  {"PY","PRY"},
  {"PE","PER"},
  {"PH","PHL"},
  {"PN","PCN"},
  {"PL","POL"},
  {"PT","PRT"},
  {"PR","PRI"},
  {"QA","QAT"},
  {"RE","REU"},
  {"RO","ROU"},
  {"RU","RUS"},
  {"RW","RWA"},
  {"BL","BLM"},
  {"SH","SHN"},
  {"KN","KNA"},
  {"LC","LCA"},
  {"MF","MAF"},
  {"PM","SPM"},
  {"VC","VCT"},
  {"WS","WSM"},
  {"SM","SMR"},
  {"ST","STP"},
  {"SA","SAU"},
  {"SN","SEN"},
  {"RS","SRB"},
  {"SC","SYC"},
  {"SL","SLE"},
  {"SG","SGP"},
  {"SX","SXM"},
  {"SK","SVK"},
  {"SI","SVN"},
  {"SB","SLB"},
  {"SO","SOM"},
  {"ZA","ZAF"},
  {"GS","SGS"},
  {"SS","SSD"},
  {"ES","ESP"},
  {"LK","LKA"},
  {"SD","SDN"},
  {"SR","SUR"},
  {"SJ","SJM"},
  {"SZ","SWZ"},
  {"SE","SWE"},
  {"CH","CHE"},
  {"SY","SYR"},
  {"TW","TWN"},
  {"TJ","TJK"},
  {"TZ","TZA"},
  {"TH","THA"},
  {"TL","TLS"},
  {"TG","TGO"},
  {"TK","TKL"},
  {"TO","TON"},
  {"TT","TTO"},
  {"TN","TUN"},
  {"TR","TUR"},
  {"TM","TKM"},
  {"TC","TCA"},
  {"TV","TUV"},
  {"UG","UGA"},
  {"UA","UKR"},
  {"AE","ARE"},
  {"GB","GBR"},
  {"US","USA"},
  {"UM","UMI"},
  {"UY","URY"},
  {"UZ","UZB"},
  {"VU","VUT"},
  {"VE","VEN"},
  {"VN","VNM"},
  {"VG","VGB"},
  {"VI","VIR"},
  {"WF","WLF"},
  {"EH","ESH"},
  {"YE","YEM"},
  {"ZM","ZMB"},
  {"ZW","ZWE"},
};

// Connector::iter() equivalent - all Connector enum variants (common_enums/src/connector_enums.rs:32).
inline const std::vector<std::string> kConnectorIter{
  "absa_sanlam",
  "aci",
  "adyen",
  "adyen_test",
  "adyenplatform",
  "affirm",
  "airwallex",
  "amazonpay",
  "archipel",
  "authipay",
  "authorizedotnet",
  "bambora",
  "bamboraapac",
  "bankofamerica",
  "barclaycard",
  "betterpayment",
  "billwerk",
  "bitpay",
  "blackhawknetwork",
  "bluesnap",
  "boku",
  "braintree",
  "breadpay",
  "calida",
  "cardinal",
  "cashtocode",
  "celero",
  "chargebee",
  "checkbook",
  "checkout",
  "checkout_test",
  "citigate",
  "coinbase",
  "coingate",
  "cryptopay",
  "ctp_mastercard",
  "ctp_visa",
  "custombilling",
  "cybersource",
  "cybersourcedecisionmanager",
  "d24",
  "datatrans",
  "deutschebank",
  "digitalvirgo",
  "dlocal",
  "dwolla",
  "ebanx",
  "ecommpay",
  "elavon",
  "elavon_pg",
  "envoy",
  "etisalat",
  "facilitapay",
  "fauxpay",
  "finix",
  "fiserv",
  "fiservcommercehub",
  "fiservemea",
  "fiuu",
  "flexiti",
  "forte",
  "getnet",
  "gigadat",
  "givepayments",
  "globalpay",
  "globalpayments_heartland",
  "globalpayments_realex",
  "globepay",
  "gocardless",
  "gotyme_sanlam",
  "gpayments",
  "helcim",
  "hipay",
  "hyperpg",
  "hyperswitch_vault",
  "iatapay",
  "ilixium",
  "imerchantsolutions",
  "inespay",
  "interpayments",
  "itaubank",
  "jpmorgan",
  "jpmorgan_orbital",
  "juspay",
  "juspaythreedsserver",
  "klarna",
  "loonio",
  "merchante",
  "mifinity",
  "mollie",
  "moneris",
  "multisafepay",
  "netcetera",
  "nexinets",
  "nexixpay",
  "nmi",
  "nomupay",
  "noon",
  "nordea",
  "novalnet",
  "nsure",
  "nuvei",
  "opennode",
  "paybox",
  "payconex",
  "paydotcom",
  "payhound",
  "payjustnow",
  "payjustnowinstore",
  "payload",
  "payme",
  "paynearme",
  "payone",
  "paypal",
  "paypal_test",
  "paysafe",
  "paystack",
  "paytm",
  "payu",
  "peachpayments",
  "phonepe",
  "phonypay",
  "placetopay",
  "plaid",
  "powertranz",
  "pretendpay",
  "prophetpay",
  "rapyd",
  "razorpay",
  "recurly",
  "redsys",
  "revolv3",
  "riskified",
  "saferpay",
  "sanlam_payshield",
  "santander",
  "shift4",
  "signifyd",
  "silverflow",
  "square",
  "stax",
  "stripe",
  "stripe_billing_test",
  "stripe_test",
  "stripebilling",
  "taxjar",
  "tesouro",
  "threedsecureio",
  "tokenex",
  "tokenio",
  "truelayer",
  "trustly",
  "trustpay",
  "trustpayments",
  "tsys",
  "tsys_transit",
  "vgs",
  "volt",
  "wellsfargo",
  "wise",
  "worldline",
  "worldpay",
  "worldpaymodular",
  "worldpayraft",
  "worldpayvantiv",
  "worldpayxml",
  "xendit",
  "zen",
  "zift",
  "zsl",
};

/**
 * Local constexpr stand-in for the upstream `all(feature = "olap", feature = "v1")` compile-time
 * route gate (crates/router/src/routes/app.rs:3430). Cargo features are not carried through the
 * C++ port; the gate is expressed here and honoured by `install()` below, with an additional
 * runtime config override for deployments that do not expose the olap/v1 API surface.
 */
inline constexpr bool kOlapV1Enabled = true;

[[nodiscard]] inline std::string to_upper(std::string_view value)
{
    std::string result{value};
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char character) {
        return static_cast<char>(std::toupper(character));
    });
    return result;
}

[[nodiscard]] inline std::vector<std::string> split_csv(std::string_view csv)
{
    std::vector<std::string> values{};
    while (!csv.empty()) {
        const auto comma = csv.find(',');
        std::string_view token = csv.substr(0, comma);
        while (!token.empty() && token.front() == ' ') {
            token.remove_prefix(1);
        }
        while (!token.empty() && token.back() == ' ') {
            token.remove_suffix(1);
        }
        if (!token.empty()) {
            values.emplace_back(token);
        }
        if (comma == std::string_view::npos) {
            break;
        }
        csv.remove_prefix(comma + 1);
    }
    return values;
}

[[nodiscard]] inline bool connector_iter_contains(std::string_view name)
{
    return std::find(kConnectorIter.begin(), kConnectorIter.end(), name) != kConnectorIter.end();
}

} // namespace detail

/**
 * @brief GET /feature_matrix domain handler.
 *
 * Upstream registers exactly one route (`fetch_feature_matrix`, `app.rs:3433-3435`, gated
 * `all(feature = "olap", feature = "v1")` at `app.rs:3430`) under `&auth::NoAuth` and
 * `LockAction::NotApplicable` (`routes/feature_matrix.rs:30-39`). All generation happens in the
 * internal helper `generate_feature_matrix` (`routes/feature_matrix.rs:42-161`), ported verbatim
 * below as `generate_feature_matrix`, `build_connector_feature_details` and
 * `build_payment_method_wise_feature_details`. There is deliberately no C++-only
 * `/generate_feature_matrix` route — that identifier is an internal upstream helper only.
 */
class FeatureMatrixRouteHandlers
{
public:
    void handle_fetch_feature_matrix(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // `#[instrument(skip_all)]` + Flow::FeatureMatrix (routes/feature_matrix.rs:19,25) and
        // api::server_wrap(..., &auth::NoAuth, LockAction::NotApplicable) (:30-38). The NO_AUTH
        // rule is a pass-through; no lock is acquired because the request is a read-only GET.
        const auto rule = m_context->auth_rules().rule_for("fetch_feature_matrix");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        m_context->metrics().record_flow("feature_matrix.fetch_feature_matrix");
        m_context->metrics().counter("feature_matrix_fetch").increment();

        generate_feature_matrix(req, res, send);
    }

    void install(core::router::RouterContext<>& router)
    {
        // Compile-time `all(feature = "olap", feature = "v1")` gate (app.rs:3430) expressed as a
        // local constexpr, plus the existing runtime config gate facility used by blocklist.
        if constexpr (!detail::kOlapV1Enabled) {
            return;
        }
        if (const auto enabled = m_context->config_store().get_value("route_features", "feature_matrix");
            enabled.has_value() && *enabled == "false") {
            return;
        }

        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/feature_matrix"}
            .add_route(
                Route{"/"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_fetch_feature_matrix(req, res, std::move(send));
                    })
                    .summary("Hyperswitch fetch_feature_matrix")
            );
    }

    explicit FeatureMatrixRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    /** @brief Port of upstream `generate_feature_matrix` (routes/feature_matrix.rs:42-73). */
    void generate_feature_matrix(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // `json_payload.map(into_inner).unwrap_or_else(|| FeatureMatrixRequest { connectors: None })`
        // (routes/feature_matrix.rs:26-28). web::Json deserializes `Vec<Connector>` as snake_case
        // strings; an unknown enum string is a 400 at the web layer, so reject it here too.
        const auto body = req.body_as_string();
        std::vector<std::string> requested{};
        if (!body.empty()) {
            auto generic = serde::Ser::decode_generic("application/json", body);
            if (!generic) {
                send_error(res, send, core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            if (const auto* object = std::get_if<interfaces::Value::Object>(&generic->get())) {
                if (const auto connectors_field = object->get("connectors")) {
                    if (const auto* array = std::get_if<interfaces::Value::Array>(&connectors_field.value().get())) {
                        for (const auto& element : *array) {
                            const auto* text = std::get_if<std::string>(&element.get());
                            if (!text || !detail::connector_iter_contains(*text)) {
                                send_error(res, send, core::error::AppErrorBuilder{}(
                                    core::error::AppErrorKind::INVALID_JSON, "invalid connector name"));
                                return;
                            }
                            requested.push_back(*text);
                        }
                    }
                }
            }
        }

        // `req.connectors.unwrap_or_else(|| Connector::iter().collect())` (:46-48). The registry
        // below is the exact strum EnumIter wire-name set from common_enums/connector_enums.rs:32.
        const std::vector<std::string> connector_list =
            requested.empty() ? detail::kConnectorIter : requested;

        // `connector_list.into_iter().filter_map(|connector_name| convert_connector(...)
        // .inspect_err(warn).ok().and_then(|c| build_connector_feature_details(..., c, ...)))`
        // (:50-65).
        std::vector<app::dto::feature_matrix::ConnectorFeatureMatrixResponse> feature_matrix_response{};
        feature_matrix_response.reserve(connector_list.size());
        for (const auto& connector_name : connector_list) {
            if (!convert_connector(connector_name)) {
                continue;
            }
            auto built = build_connector_feature_details(connector_name);
            if (built) {
                feature_matrix_response.push_back(std::move(*built));
            }
        }

        // `Ok(ApplicationResponse::Json(FeatureMatrixListResponse { connector_count, connectors }))`
        // (:67-72).
        app::dto::feature_matrix::FeatureMatrixListResponse response;
        response.set_connector_count(feature_matrix_response.size());
        response.set_connectors(std::move(feature_matrix_response));

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    /**
     * @brief Equivalent of `api_types::feature_matrix::FeatureMatrixConnectorData::convert_connector`
     * (router/src/types/api/feature_matrix.rs:16-522) followed by the `inspect_err(warn!)` at
     * routes/feature_matrix.rs:54-60. Cardinal / Juspay / Nsure are valid enum members but the
     * upstream conversion deliberately returns `InvalidConnectorName` for them (:512-516).
     */
    [[nodiscard]] bool convert_connector(std::string_view name) const
    {
        static const std::unordered_set<std::string_view> kRejected{"cardinal", "juspay", "nsure"};
        if (!detail::connector_iter_contains(name) || kRejected.contains(name)) {
            // router_env::logger::warn!("Failed to fetch {:?} details", connector_name)
            std::fprintf(stderr, "warn: Failed to fetch \"%.*s\" details\n",
                         static_cast<int>(name.size()), name.data());
            m_context->metrics().counter("feature_matrix.convert_connector.failed").increment();
            return false;
        }
        return true;
    }

    /**
     * @brief Port of `build_connector_feature_details` (routes/feature_matrix.rs:75-112).
     *
     * Returns `std::nullopt` where upstream's `connector_about.map(...)` yields `None`
     * (`get_connector_about()` returns `None` — connector absent from the catalog). This drops
     * connectors such as ctp_mastercard / vgs / recurly exactly as upstream does.
     */
    [[nodiscard]] std::optional<app::dto::feature_matrix::ConnectorFeatureMatrixResponse>
    build_connector_feature_details(std::string_view name) const
    {
        using namespace app::dto::feature_matrix;

        const auto it = detail::catalog::kEntries.find(std::string{name});
        if (it == detail::catalog::kEntries.end()) {
            return std::nullopt; // connector.get_connector_about() == None
        }
        const auto& entry = it->second;

        // `let connector_integration_features = connector.get_supported_payment_methods();`
        // `let supported_payment_methods = connector_integration_features.map(|data| ...
        //   .flat_map(|(pm, types)| build_payment_method_wise_feature_details(...)).collect());`
        // (:80-94). The catalog row set is the transcribed `SupportedPaymentMethods` table.
        std::vector<SupportedPaymentMethod> methods{};
        for (const auto& row : entry.methods) {
            auto built = build_payment_method_wise_feature_details(name, row);
            methods.insert(methods.end(), std::make_move_iterator(built.begin()),
                           std::make_move_iterator(built.end()));
        }

        // `let supported_webhook_flows = connector.get_supported_webhook_flows()
        //   .map(|flows| flows.to_vec());` (:95-97).
        std::vector<std::string> flows{};
        flows.reserve(entry.webhooks.size());
        for (const auto flow : entry.webhooks) {
            flows.emplace_back(flow);
        }

        ConnectorFeatureMatrixResponse out;
        out.set_name(detail::to_upper(name));                          // :102
        out.set_display_name(std::string{entry.display_name});         // :103
        out.set_description(std::string{entry.description});           // :104
        // `base_url: Some(connector.base_url(&state.conf.connectors).to_string())` (:105). The
        // config_store namespace is this port's `state.conf.connectors`.
        out.set_base_url(m_context->config_store()
                             .get_value("connector_base_url", name)
                             .value_or(std::format("https://api.{}.com", name)));
        out.set_integration_status(std::string{entry.integration_status}); // :106
        out.set_category(std::string{entry.category});                 // :107
        out.set_supported_webhook_flows(std::move(flows));             // :108
        out.set_supported_payment_methods(std::move(methods));         // :109
        return out;
    }

    /**
     * @brief Port of `build_payment_method_wise_feature_details`
     * (routes/feature_matrix.rs:114-161).
     *
     * The `state.conf.pm_filters` lookups (:123-146) are served by the config_store: a
     * `pm_filters` record keyed `<connector>.<payment_method_type>.country` / `.currency`.
     * Configured countries are alpha-2 codes converted through the transcribed
     * `CountryAlpha2::from_alpha2_to_alpha3` map (common_enums/src/transformers.rs:23).
     */
    [[nodiscard]] std::vector<app::dto::feature_matrix::SupportedPaymentMethod>
    build_payment_method_wise_feature_details(
        std::string_view connector_name,
        const detail::catalog::FeatureRow& row
    ) const
    {
        using namespace app::dto::feature_matrix;

        const std::string filter_key = std::string{connector_name} + "." + std::string{row.pmt};

        // `payment_method_type_config = state.conf.pm_filters.0.get(connector_name)
        //   .and_then(|sel| sel.0.get(&PaymentMethodFilterKey::PaymentMethodType(pmt)))` (:123-135)
        std::optional<std::vector<std::string>> supported_countries{};
        if (const auto raw = m_context->config_store().get_value("pm_filters", filter_key + ".country")) {
            const auto codes = detail::split_csv(*raw);
            std::set<std::string> unique{};
            for (const auto& code : codes) {
                const auto converted = detail::kAlpha2ToAlpha3.find(code);
                unique.insert(converted != detail::kAlpha2ToAlpha3.end()
                                  ? std::string{converted->second}
                                  : code);
            }
            supported_countries = std::vector<std::string>{unique.begin(), unique.end()};
        }

        std::optional<std::vector<std::string>> supported_currencies{};
        if (const auto raw = m_context->config_store().get_value("pm_filters", filter_key + ".currency")) {
            const auto codes = detail::split_csv(*raw);
            const std::set<std::string> unique{codes.begin(), codes.end()};
            supported_currencies = std::vector<std::string>{unique.begin(), unique.end()};
        }

        SupportedPaymentMethod item;
        item.set_payment_method(std::string{row.pm});                              // :149
        item.set_payment_method_type(std::string{row.pmt});                        // :150
        item.set_payment_method_type_display_name(std::string{row.display});       // :151
        item.set_mandates(std::string{row.mandates});                              // :152
        item.set_refunds(std::string{row.refunds});                                // :153
        std::vector<std::string> captures{};
        captures.reserve(row.captures.size());
        for (const auto capture : row.captures) {
            captures.emplace_back(capture);
        }
        item.set_supported_capture_methods(std::move(captures));                  // :154
        if (row.has_card) {
            // `specific_features: feature_metadata.specific_features` (:155). Upstream's
            // #[serde(flatten)] PaymentMethodSpecificFeatures::Card emits three_ds, no_three_ds
            // and supported_card_networks at the row's top level; rfl omits the empty optionals
            // for every non-card row, matching the None case.
            item.set_three_ds(std::string{row.three_ds});
            item.set_no_three_ds(std::string{row.no_three_ds});
            std::vector<std::string> networks{};
            networks.reserve(row.networks.size());
            for (const auto network : row.networks) {
                networks.emplace_back(network);
            }
            item.set_supported_card_networks(std::move(networks));
        }
        item.set_supported_countries(std::move(supported_countries));             // :156
        item.set_supported_currencies(std::move(supported_currencies));           // :157

        return std::vector<SupportedPaymentMethod>{std::move(item)};
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::feature_matrix
