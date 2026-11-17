/**
 * Ghost Blog Plugin - Posts Service
 * Mirrors Ghost's posts service for API compatibility
 */
import * as knex from 'knex';
import { Post, ApiResponse } from '../types';
export interface PostsService {
    browsePosts(options: any): Promise<ApiResponse<Post[]>>;
    readPost(frame: any): Promise<ApiResponse<Post>>;
    addPost(data: Post, options: any): Promise<Post>;
    editPost(frame: any, options: {
        eventHandler?: (event: string, dto: Post) => void;
    }): Promise<Post>;
    bulkEdit(bulkData: any, options: any): Promise<any>;
    bulkDestroy(options: any): Promise<any>;
    destroyPost(data: any, options: any): Promise<void>;
    copyPost(frame: any): Promise<Post>;
    exportPosts(options: any): Promise<any>;
    importCSV(data: any): Promise<any>;
}
export declare function createPostsService(knexInstance: knex.Knex, bookshelfInstance: any): PostsService;
export declare const getPostServiceInstance: () => (knexInstance?: knex.Knex, bookshelfInstance?: any) => PostsService;
//# sourceMappingURL=posts-service.d.ts.map