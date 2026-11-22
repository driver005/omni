// Mock Hyperswitch API for UI testing (the C++ port has no runnable server binary yet).
// Serves representative wire shapes on port 8080; /api is stripped by the UI's dev proxy.
const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = process.env.PORT || 8080;
const DIST = path.join(__dirname, 'dist', 'hyperswitch');

const now = () => new Date().toISOString();
const merchant = {
  merchant_id: 'merchant_123',
  merchant_name: 'Test Merchant',
  return_url: 'http://localhost:9000/return',
  webhook_details: { webhook_url: 'http://localhost:9000/webhook' },
  routing_algorithm: null,
  offline_payment_methods: [],
  created_at: now(),
};
const payment = (id, amount) => ({
  payment_id: id,
  status: 'succeeded',
  amount: amount,
  currency: 'USD',
  description: 'Test payment',
  customer_id: 'customer_1',
  profile_id: 'pro_1',
  connector: 'stripe',
  payment_method: 'card',
  created_at: now(),
  metadata: {},
  next_action: null,
});
const payments = [
  payment('pay_1', 6540, 'USD'),
  payment('pay_2', 12900, 'USD'),
  payment('pay_3', 3250, 'USD'),
];

const routes = {
  '/health': () => ({ status: 'ok' }),
  '/payments/list': () => ({ data: payments, meta: { page: 1, limit: 10, total: payments.length } }),
  '/payments': () => ({ data: payments, meta: { page: 1, limit: 10, total: payments.length } }),
  '/customers/list': () => ({ data: [{ customer_id: 'customer_1', email: 'test@example.com', name: 'Test User' }], meta: { page: 1, total: 1 } }),
  '/refunds/list': () => ({ data: [], meta: { page: 1, total: 0 } }),
  '/payouts/list': () => ({ data: [], meta: { page: 1, total: 0 } }),
  '/payment-methods/list': () => ({ data: [], meta: { total: 0 } }),
  '/connectors': () => ({ data: [{ connector_name: 'stripe', connector_type: 'payment_processor', disabled: false }] }),
  '/api-keys/list': () => ({ data: [], meta: { total: 0 } }),
  '/organizations': () => ({ data: [{ organization_id: 'org_1', organization_name: 'Test Org' }] }),
  '/merchants': () => ({ data: [merchant] }),
  '/profiles/list': () => ({ data: [{ profile_id: 'pro_1', profile_name: 'default', merchant_id: 'merchant_123' }] }),
  '/disputes/list': () => ({ data: [], meta: { total: 0 } }),
  '/webhooks/events/list': () => ({ data: [], meta: { total: 0 } }),
  '/routing/list': () => ({ data: [], meta: { total: 0 } }),
  '/feature_matrix': () => ({ connectors: [] }),
};

const server = http.createServer((req, res) => {
  const url = new URL(req.url, `http://${req.headers.host}`);
  let p = url.pathname;
  if (p.startsWith('/api')) p = p.slice(4);

  // CORS
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Headers', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET,POST,PUT,DELETE,OPTIONS');
  if (req.method === 'OPTIONS') { res.writeHead(204); res.end(); return; }

  // config feature endpoint used by dashboard bootstrap
  if (p.startsWith('/config/')) {
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({})); return;
  }

  const handler = routes[p];
  if (handler) {
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify(handler()));
    return;
  }
  // default empty success for unknown API routes
  if (p.startsWith('/')) {
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ data: [], meta: { page: 1, total: 0 } }));
    return;
  }
  res.writeHead(404); res.end();
});

server.listen(PORT, '0.0.0.0', () => console.log(`Mock Hyperswitch API on :${PORT}`));
