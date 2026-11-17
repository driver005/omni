"use strict";
/**
 * Ghost Blog Plugin - Main Entry Point
 * Drop-in replacement for Ghost backend API
 */
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.postsService = exports.models = exports.bookshelfInstance = exports.knexInstance = exports.app = void 0;
const express_1 = __importDefault(require("express"));
const body_parser_1 = __importDefault(require("body-parser"));
const cors_1 = __importDefault(require("cors"));
const models_1 = require("./models");
const posts_service_1 = require("./services/posts-service");
// Configuration
const config = {
    client: process.env.DB_CLIENT || 'sqlite3',
    connection: {
        filename: process.env.DB_FILENAME || './ghost.db'
    },
    pool: {
        min: 2,
        max: 10
    },
    migrations: {
        tableName: 'knex_migrations',
        directory: './migrations'
    }
};
// Initialize database
const knexInstance = (0, models_1.createKnex)(config);
exports.knexInstance = knexInstance;
const bookshelfInstance = (0, models_1.createBookshelf)(knexInstance);
exports.bookshelfInstance = bookshelfInstance;
const models = (0, models_1.initModels)(bookshelfInstance);
exports.models = models;
const postsService = (0, posts_service_1.createPostsService)(knexInstance, bookshelfInstance);
exports.postsService = postsService;
// Express app
const app = (0, express_1.default)();
exports.app = app;
app.use((0, cors_1.default)());
app.use(body_parser_1.default.json({ limit: '50mb' }));
app.use(body_parser_1.default.urlencoded({ extended: true, limit: '50mb' }));
// Error handler helper
function handleError(res, error, statusCode = 500) {
    const message = error instanceof Error ? error.message : 'Internal server error';
    res.status(statusCode).json({ success: false, error: message });
}
// Health check
app.get('/health', (req, res) => {
    res.json({ status: 'ok', timestamp: new Date().toISOString() });
});
// ============================================
// ADMIN API ROUTES (/admin/)
// ============================================
const adminRouter = express_1.default.Router();
// Posts Admin API
adminRouter.get('/posts/', async (req, res) => {
    try {
        const options = {
            filter: req.query.filter,
            limit: parseInt(req.query.limit) || 15,
            order: req.query.order,
            page: parseInt(req.query.page) || 1,
            include: req.query.include,
            fields: req.query.fields,
            formats: req.query.formats
        };
        const result = await postsService.browsePosts(options);
        res.status(result.status_code || 200).json(result);
    }
    catch (error) {
        handleError(res, error);
    }
});
adminRouter.get('/posts/:id/', async (req, res) => {
    try {
        const frame = {
            options: {
                id: req.params.id,
                include: req.query.include,
                fields: req.query.fields,
                formats: req.query.formats
            }
        };
        const result = await postsService.readPost(frame);
        res.status(result.status_code || 200).json(result);
    }
    catch (error) {
        handleError(res, error);
    }
});
adminRouter.post('/posts/', async (req, res) => {
    try {
        const postData = req.body.posts[0];
        const options = {
            include: req.query.include,
            formats: req.query.formats,
            source: req.query.source
        };
        const post = await postsService.addPost(postData, options);
        res.status(201).json({ success: true, data: post });
    }
    catch (error) {
        handleError(res, error);
    }
});
adminRouter.put('/posts/:id/', async (req, res) => {
    try {
        const frame = {
            data: { posts: [req.body] },
            options: {
                id: req.params.id,
                include: req.query.include,
                formats: req.query.formats,
                source: req.query.source,
                email_segment: req.query.email_segment,
                newsletter: req.query.newsletter,
                force_rerender: req.query.force_rerender === 'true',
                save_revision: req.query.save_revision === 'true'
            }
        };
        const post = await postsService.editPost(frame, {
            eventHandler: (event, dto) => {
                // Handle cache invalidation
            }
        });
        res.json({ success: true, data: post });
    }
    catch (error) {
        handleError(res, error);
    }
});
adminRouter.delete('/posts/:id/', async (req, res) => {
    try {
        await postsService.destroyPost({}, { id: req.params.id, require: true });
        res.status(204).send();
    }
    catch (error) {
        handleError(res, error, 404);
    }
});
adminRouter.post('/posts/:id/copy/', async (req, res) => {
    try {
        const frame = {
            options: {
                id: req.params.id,
                formats: req.query.formats
            }
        };
        const post = await postsService.copyPost(frame);
        res.status(201).json({ success: true, data: post });
    }
    catch (error) {
        handleError(res, error);
    }
});
adminRouter.delete('/posts/', async (req, res) => {
    try {
        const result = await postsService.bulkDestroy({ filter: req.query.filter });
        res.json({ success: true, data: result });
    }
    catch (error) {
        handleError(res, error);
    }
});
adminRouter.put('/posts/bulk/', async (req, res) => {
    try {
        const result = await postsService.bulkEdit(req.body.bulk, { filter: req.query.filter });
        res.json({ success: true, data: result });
    }
    catch (error) {
        handleError(res, error);
    }
});
adminRouter.get('/posts/export/', async (req, res) => {
    try {
        const result = await postsService.exportPosts({
            filter: req.query.filter,
            limit: parseInt(req.query.limit) || 15,
            order: req.query.order
        });
        res.setHeader('Content-Type', 'text/csv');
        res.setHeader('Content-Disposition', `attachment; filename="${result.filename}"`);
        res.send(result.data);
    }
    catch (error) {
        handleError(res, error);
    }
});
// Tags Admin API
adminRouter.get('/tags/', async (req, res) => {
    try {
        const TagModel = models.Tag;
        const options = {
            limit: parseInt(req.query.limit) || 15,
            order: req.query.order || 'name ASC',
            page: parseInt(req.query.page) || 1,
            include: req.query.include,
            fields: req.query.fields
        };
        let query = TagModel.forge().query((qb) => {
            qb.select('*');
            qb.orderBy('name', 'ASC');
            qb.limit(options.limit).offset((options.page - 1) * options.limit);
        });
        const tags = await query.fetchAll({ withRelated: options.include?.split(',') || [] });
        res.json({ success: true, data: tags.toJSON() });
    }
    catch (error) {
        handleError(res, error);
    }
});
adminRouter.get('/tags/:id/', async (req, res) => {
    try {
        const TagModel = models.Tag;
        const tag = await TagModel.where({ id: req.params.id }).fetch({ require: true });
        res.json({ success: true, data: tag.toJSON() });
    }
    catch (error) {
        handleError(res, error, 404);
    }
});
adminRouter.post('/tags/', async (req, res) => {
    try {
        const TagModel = models.Tag;
        const tag = TagModel.forge(req.body);
        const saved = await tag.save(null, { method: 'insert' });
        res.status(201).json({ success: true, data: saved.toJSON() });
    }
    catch (error) {
        handleError(res, error);
    }
});
adminRouter.put('/tags/:id/', async (req, res) => {
    try {
        const TagModel = models.Tag;
        const tag = await TagModel.where({ id: req.params.id }).fetch({ require: true });
        const saved = await tag.save(req.body, { method: 'update', patch: true });
        res.json({ success: true, data: saved.toJSON() });
    }
    catch (error) {
        handleError(res, error);
    }
});
adminRouter.delete('/tags/:id/', async (req, res) => {
    try {
        const TagModel = models.Tag;
        const tag = await TagModel.where({ id: req.params.id }).fetch({ require: true });
        await tag.destroy();
        res.status(204).send();
    }
    catch (error) {
        handleError(res, error, 404);
    }
});
// Authors (Users) Admin API
adminRouter.get('/users/', async (req, res) => {
    try {
        const AuthorModel = models.Author;
        const options = {
            limit: parseInt(req.query.limit) || 15,
            order: req.query.order || 'name ASC',
            page: parseInt(req.query.page) || 1,
            include: req.query.include
        };
        let query = AuthorModel.forge().query((qb) => {
            qb.select('*');
            qb.orderBy('name', 'ASC');
            qb.limit(options.limit).offset((options.page - 1) * options.limit);
        });
        const authors = await query.fetchAll({ withRelated: options.include?.split(',') || [] });
        res.json({ success: true, data: authors.toJSON() });
    }
    catch (error) {
        handleError(res, error);
    }
});
adminRouter.get('/users/:id/', async (req, res) => {
    try {
        const AuthorModel = models.Author;
        const author = await AuthorModel.where({ id: req.params.id }).fetch({ require: true });
        res.json({ success: true, data: author.toJSON() });
    }
    catch (error) {
        handleError(res, error, 404);
    }
});
adminRouter.post('/users/', async (req, res) => {
    try {
        const AuthorModel = models.Author;
        const author = AuthorModel.forge(req.body);
        const saved = await author.save(null, { method: 'insert' });
        res.status(201).json({ success: true, data: saved.toJSON() });
    }
    catch (error) {
        handleError(res, error);
    }
});
adminRouter.put('/users/:id/', async (req, res) => {
    try {
        const AuthorModel = models.Author;
        const author = await AuthorModel.where({ id: req.params.id }).fetch({ require: true });
        const saved = await author.save(req.body, { method: 'update', patch: true });
        res.json({ success: true, data: saved.toJSON() });
    }
    catch (error) {
        handleError(res, error);
    }
});
adminRouter.delete('/users/:id/', async (req, res) => {
    try {
        const AuthorModel = models.Author;
        const author = await AuthorModel.where({ id: req.params.id }).fetch({ require: true });
        await author.destroy();
        res.status(204).send();
    }
    catch (error) {
        handleError(res, error, 404);
    }
});
// Members Admin API
adminRouter.get('/members/', async (req, res) => {
    try {
        const MemberModel = models.Member;
        const options = {
            limit: parseInt(req.query.limit) || 15,
            order: req.query.order || 'created_at DESC',
            page: parseInt(req.query.page) || 1,
            include: req.query.include
        };
        let query = MemberModel.forge().query((qb) => {
            qb.select('*');
            qb.orderBy('created_at', 'DESC');
            qb.limit(options.limit).offset((options.page - 1) * options.limit);
        });
        const members = await query.fetchAll({ withRelated: options.include?.split(',') || [] });
        res.json({ success: true, data: members.toJSON() });
    }
    catch (error) {
        handleError(res, error);
    }
});
// Pages Admin API
adminRouter.get('/pages/', async (req, res) => {
    try {
        const PostModel = models.Post;
        const options = {
            limit: parseInt(req.query.limit) || 15,
            order: req.query.order || 'published_at DESC',
            page: parseInt(req.query.page) || 1,
            include: req.query.include,
            filter: `type:page,${req.query.filter || ''}`
        };
        let query = PostModel.forge().query((qb) => {
            qb.select('*').where('type', 'page');
            qb.orderBy('published_at', 'DESC');
            qb.limit(options.limit).offset((options.page - 1) * options.limit);
        });
        const pages = await query.fetchAll({ withRelated: options.include?.split(',') || [] });
        res.json({ success: true, data: pages.toJSON() });
    }
    catch (error) {
        handleError(res, error);
    }
});
// Newsletters Admin API
adminRouter.get('/newsletters/', async (req, res) => {
    try {
        const NewsletterModel = models.Newsletter;
        const options = {
            limit: parseInt(req.query.limit) || 15,
            order: req.query.order || 'created_at DESC',
            page: parseInt(req.query.page) || 1
        };
        let query = NewsletterModel.forge().query((qb) => {
            qb.select('*');
            qb.orderBy('created_at', 'DESC');
            qb.limit(options.limit).offset((options.page - 1) * options.limit);
        });
        const newsletters = await query.fetchAll();
        res.json({ success: true, data: newsletters.toJSON() });
    }
    catch (error) {
        handleError(res, error);
    }
});
// Tiers Admin API
adminRouter.get('/tiers/', async (req, res) => {
    try {
        const TierModel = models.Tier;
        const options = {
            limit: parseInt(req.query.limit) || 15,
            order: req.query.order || 'monthly_price ASC',
            page: parseInt(req.query.page) || 1
        };
        let query = TierModel.forge().query((qb) => {
            qb.select('*');
            qb.orderBy('monthly_price', 'ASC');
            qb.limit(options.limit).offset((options.page - 1) * options.limit);
        });
        const tiers = await query.fetchAll();
        res.json({ success: true, data: tiers.toJSON() });
    }
    catch (error) {
        handleError(res, error);
    }
});
// Settings Admin API
adminRouter.get('/settings/', async (req, res) => {
    try {
        const SettingsModel = models.Settings;
        const settings = await SettingsModel.forge().fetchAll();
        res.json({ success: true, data: settings.toJSON() });
    }
    catch (error) {
        handleError(res, error);
    }
});
adminRouter.put('/settings/', async (req, res) => {
    try {
        const SettingsModel = models.Settings;
        const updates = req.body;
        for (const [key, value] of Object.entries(updates)) {
            await SettingsModel.where({ key }).save({ value: JSON.stringify(value) }, { method: 'update' });
        }
        res.json({ success: true, data: updates });
    }
    catch (error) {
        handleError(res, error);
    }
});
// Site Admin API
adminRouter.get('/site/', async (req, res) => {
    try {
        const SettingsModel = models.Settings;
        const siteSettings = await SettingsModel.where({
            key: ['title', 'description', 'logo', 'url', 'timezone', 'locale', 'language']
        }).fetchAll();
        const site = {
            id: '1',
            uuid: 'site-uuid',
            title: '',
            description: '',
            logo: '',
            url: '',
            version: '1.0.0',
            timezone: 'UTC',
            locale: 'en_US',
            language: 'en',
            ghost_version: '5.0.0',
            environment: process.env.NODE_ENV || 'development',
            meta: {}
        };
        siteSettings.toJSON().forEach((s) => {
            if (s.key in site) {
                site[s.key] = JSON.parse(s.value);
            }
        });
        res.json({ success: true, data: site });
    }
    catch (error) {
        handleError(res, error);
    }
});
app.use('/admin/', adminRouter);
// ============================================
// CONTENT API ROUTES (/content/)
// ============================================
const contentRouter = express_1.default.Router();
// Posts Content API
contentRouter.get('/posts/', async (req, res) => {
    try {
        const options = {
            filter: req.query.filter,
            limit: parseInt(req.query.limit) || 15,
            order: req.query.order || 'published_at DESC',
            page: parseInt(req.query.page) || 1,
            include: req.query.include,
            fields: req.query.fields,
            formats: req.query.formats,
            absolute_urls: req.query.absolute_urls === 'true',
            collection: req.query.collection
        };
        // Force published status for public API
        const filter = options.filter ? `${options.filter},status:published` : 'status:published';
        const result = await postsService.browsePosts({ ...options, filter });
        // Apply visibility filtering for public API
        if (result.data) {
            result.data = result.data.filter(post => post.visibility === 'public' || post.visibility === 'members');
        }
        res.status(result.status_code || 200).json(result);
    }
    catch (error) {
        handleError(res, error);
    }
});
contentRouter.get('/posts/:id/', async (req, res) => {
    try {
        const frame = {
            options: {
                id: req.params.id,
                include: req.query.include,
                fields: req.query.fields,
                formats: req.query.formats,
                absolute_urls: req.query.absolute_urls === 'true'
            }
        };
        const result = await postsService.readPost(frame);
        // Check visibility for public API
        if (result.data && result.data.visibility === 'paid') {
            return res.status(403).json({ success: false, error: 'Access denied', access: false });
        }
        res.status(result.status_code || 200).json(result);
    }
    catch (error) {
        handleError(res, error, 404);
    }
});
contentRouter.get('/posts/slug/:slug/', async (req, res) => {
    try {
        const frame = {
            options: {
                slug: req.params.slug,
                include: req.query.include,
                fields: req.query.fields,
                formats: req.query.formats,
                absolute_urls: req.query.absolute_urls === 'true'
            }
        };
        const result = await postsService.readPost(frame);
        if (result.data && result.data.visibility === 'paid') {
            return res.status(403).json({ success: false, error: 'Access denied', access: false });
        }
        res.status(result.status_code || 200).json(result);
    }
    catch (error) {
        handleError(res, error, 404);
    }
});
// Tags Content API
contentRouter.get('/tags/', async (req, res) => {
    try {
        const TagModel = models.Tag;
        const options = {
            limit: parseInt(req.query.limit) || 15,
            order: req.query.order || 'name ASC',
            page: parseInt(req.query.page) || 1
        };
        let query = TagModel.forge().query((qb) => {
            qb.select('*').where('visibility', 'public');
            qb.orderBy('name', 'ASC');
            qb.limit(options.limit).offset((options.page - 1) * options.limit);
        });
        const tags = await query.fetchAll();
        res.json({ success: true, data: tags.toJSON() });
    }
    catch (error) {
        handleError(res, error);
    }
});
contentRouter.get('/tags/:id/', async (req, res) => {
    try {
        const TagModel = models.Tag;
        const tag = await TagModel.where({ id: req.params.id, visibility: 'public' }).fetch({ require: true });
        res.json({ success: true, data: tag.toJSON() });
    }
    catch (error) {
        handleError(res, error, 404);
    }
});
contentRouter.get('/tags/slug/:slug/', async (req, res) => {
    try {
        const TagModel = models.Tag;
        const tag = await TagModel.where({ slug: req.params.slug, visibility: 'public' }).fetch({ require: true });
        res.json({ success: true, data: tag.toJSON() });
    }
    catch (error) {
        handleError(res, error, 404);
    }
});
contentRouter.get('/tags/:id/posts/', async (req, res) => {
    try {
        const PostModel = models.Post;
        const options = {
            limit: parseInt(req.query.limit) || 15,
            order: req.query.order || 'published_at DESC',
            page: parseInt(req.query.page) || 1,
            filter: `tag:${req.params.id},status:published`
        };
        const result = await postsService.browsePosts(options);
        res.status(result.status_code || 200).json(result);
    }
    catch (error) {
        handleError(res, error);
    }
});
// Authors Content API
contentRouter.get('/authors/', async (req, res) => {
    try {
        const AuthorModel = models.Author;
        const options = {
            limit: parseInt(req.query.limit) || 15,
            order: req.query.order || 'name ASC',
            page: parseInt(req.query.page) || 1
        };
        let query = AuthorModel.forge().query((qb) => {
            qb.select('*').where('status', 'active');
            qb.orderBy('name', 'ASC');
            qb.limit(options.limit).offset((options.page - 1) * options.limit);
        });
        const authors = await query.fetchAll();
        res.json({ success: true, data: authors.toJSON() });
    }
    catch (error) {
        handleError(res, error);
    }
});
contentRouter.get('/authors/:id/', async (req, res) => {
    try {
        const AuthorModel = models.Author;
        const author = await AuthorModel.where({ id: req.params.id, status: 'active' }).fetch({ require: true });
        res.json({ success: true, data: author.toJSON() });
    }
    catch (error) {
        handleError(res, error, 404);
    }
});
contentRouter.get('/authors/slug/:slug/', async (req, res) => {
    try {
        const AuthorModel = models.Author;
        const author = await AuthorModel.where({ slug: req.params.slug, status: 'active' }).fetch({ require: true });
        res.json({ success: true, data: author.toJSON() });
    }
    catch (error) {
        handleError(res, error, 404);
    }
});
contentRouter.get('/authors/:id/posts/', async (req, res) => {
    try {
        const options = {
            limit: parseInt(req.query.limit) || 15,
            order: req.query.order || 'published_at DESC',
            page: parseInt(req.query.page) || 1,
            filter: `author:${req.params.id},status:published`
        };
        const result = await postsService.browsePosts(options);
        res.status(result.status_code || 200).json(result);
    }
    catch (error) {
        handleError(res, error);
    }
});
// Tiers Content API
contentRouter.get('/tiers/', async (req, res) => {
    try {
        const TierModel = models.Tier;
        const tiers = await TierModel.forge().where({ active: true, visibility: 'public' }).fetchAll();
        res.json({ success: true, data: tiers.toJSON() });
    }
    catch (error) {
        handleError(res, error);
    }
});
// Settings Content API
contentRouter.get('/settings/', async (req, res) => {
    try {
        const SettingsModel = models.Settings;
        const publicSettings = await SettingsModel.where({ public: true }).fetchAll();
        res.json({ success: true, data: publicSettings.toJSON() });
    }
    catch (error) {
        handleError(res, error);
    }
});
// Newsletters Content API
contentRouter.get('/newsletters/', async (req, res) => {
    try {
        const NewsletterModel = models.Newsletter;
        const newsletters = await NewsletterModel.forge().where({ status: 'active' }).fetchAll();
        res.json({ success: true, data: newsletters.toJSON() });
    }
    catch (error) {
        handleError(res, error);
    }
});
app.use('/content/', contentRouter);
// ============================================
// MEMBERS API ROUTES (/members/)
// ============================================
const membersRouter = express_1.default.Router();
membersRouter.get('/members/', async (req, res) => {
    try {
        const MemberModel = models.Member;
        const members = await MemberModel.forge().where({ status: ['free', 'paid', 'comp'] }).fetchAll({ withRelated: ['subscriptions'] });
        res.json({ success: true, data: members.toJSON() });
    }
    catch (error) {
        handleError(res, error);
    }
});
membersRouter.get('/account/', async (req, res) => {
    // Requires authentication - placeholder
    res.json({ success: true, data: { message: 'Authentication required' } });
});
membersRouter.put('/account/', async (req, res) => {
    handleError(res, new Error('Authentication required'), 401);
});
membersRouter.delete('/account/', async (req, res) => {
    handleError(res, new Error('Authentication required'), 401);
});
membersRouter.post('/account/signin_urls/', async (req, res) => {
    res.json({ success: true, data: { signin_url: 'https://example.com/signin' } });
});
membersRouter.post('/account/commenting/disable/', async (req, res) => {
    handleError(res, new Error('Authentication required'), 401);
});
membersRouter.post('/account/commenting/enable/', async (req, res) => {
    handleError(res, new Error('Authentication required'), 401);
});
membersRouter.delete('/account/sessions/', async (req, res) => {
    handleError(res, new Error('Authentication required'), 401);
});
membersRouter.delete('/account/suppression/', async (req, res) => {
    handleError(res, new Error('Authentication required'), 401);
});
app.use('/members/', membersRouter);
// ============================================
// START SERVER
// ============================================
const PORT = parseInt(process.env.PORT || '2368', 10);
const HOST = process.env.HOST || '0.0.0.0';
async function start() {
    try {
        // Run migrations
        await knexInstance.migrate.latest();
        // Start server
        app.listen(PORT, HOST, () => {
            console.log(`Ghost Blog Plugin running on http://${HOST}:${PORT}`);
            console.log(`Admin API: http://${HOST}:${PORT}/admin/`);
            console.log(`Content API: http://${HOST}:${PORT}/content/`);
            console.log(`Members API: http://${HOST}:${PORT}/members/`);
        });
    }
    catch (error) {
        console.error('Failed to start server:', error);
        process.exit(1);
    }
}
if (require.main === module) {
    start();
}
//# sourceMappingURL=index.js.map