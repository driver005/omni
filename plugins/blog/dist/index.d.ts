/**
 * Ghost Blog Plugin - Main Entry Point
 * Drop-in replacement for Ghost backend API
 */
declare const knexInstance: any;
declare const bookshelfInstance: any;
declare const models: {
    Post: any;
    Tag: any;
    Author: any;
    Newsletter: any;
    Tier: any;
    Member: any;
    PostRevision: any;
    Settings: any;
};
declare const postsService: import("./services/posts-service").PostsService;
declare const app: import("express-serve-static-core").Express;
export { app, knexInstance, bookshelfInstance, models, postsService };
//# sourceMappingURL=index.d.ts.map