/**
 * Ghost Blog Plugin - Posts Service
 * Mirrors Ghost's posts service for API compatibility
 */

import * as knex from 'knex';
import { Post, Tag, Author, PaginationParams, PostFilter, ApiResponse } from '../types';
import { initModels } from '../models';

export interface PostsService {
  browsePosts(options: any): Promise<ApiResponse<Post[]>>;
  readPost(frame: any): Promise<ApiResponse<Post>>;
  addPost(data: Post, options: any): Promise<Post>;
  editPost(frame: any, options: { eventHandler?: (event: string, dto: Post) => void }): Promise<Post>;
  bulkEdit(bulkData: any, options: any): Promise<any>;
  bulkDestroy(options: any): Promise<any>;
  destroyPost(data: any, options: any): Promise<void>;
  copyPost(frame: any): Promise<Post>;
  exportPosts(options: any): Promise<any>;
  importCSV(data: any): Promise<any>;
}

export function createPostsService(knexInstance: knex.Knex, bookshelfInstance: any): PostsService {
  const models = initModels(bookshelfInstance);
  const PostModel = models.Post;
  const TagModel = models.Tag;
  const AuthorModel = models.Author;
  
  // Helper functions
  function parseFilter(filterStr: string): PostFilter {
    const filter: PostFilter = {};
    if (!filterStr) return filter;
    
    const parts = filterStr.split(',');
    for (const part of parts) {
      const [key, value] = part.split(':');
      if (key && value) {
        (filter as any)[key] = value.replace(/'/g, '');
      }
    }
    return filter;
  }
  
  function applyFilter(query: any, filter: PostFilter) {
    if (filter.status) {
      query.where('status', filter.status);
    }
    if (filter.author_id) {
      query.where('author_id', filter.author_id);
    }
    if (filter.tag) {
      query.whereExists(function(this: any) {
        this.select('*')
          .from('posts_tags')
          .whereRaw('posts_tags.post_id = posts.id')
          .where('tag_id', filter.tag);
      });
    }
    if (filter.search) {
      query.where(function(this: any) {
        this.where('title', 'ILIKE', `%${filter.search}%`)
          .orWhere('content', 'ILIKE', `%${filter.search}%`)
          .orWhere('plaintext', 'ILIKE', `%${filter.search}%`);
      });
    }
    if (filter.visibility) {
      query.where('visibility', filter.visibility);
    }
  }
  
  function applyPagination(query: any, params: PaginationParams) {
    const page = params.page || 1;
    const limit = params.limit || 15;
    const offset = (page - 1) * limit;
    
    query.limit(limit).offset(offset);
    
    if (params.order) {
      const [field, direction] = params.order.split(' ');
      query.orderBy(field, direction || 'DESC');
    } else {
      query.orderBy('published_at', 'DESC');
    }
  }
  
  async function browsePosts(options: any): Promise<ApiResponse<Post[]>> {
    const { filter, limit, order, page, include, fields, formats } = options;
    
    let query = PostModel.forge().query((qb: any) => {
      qb.select('*');
      
      if (filter) {
        const parsedFilter = typeof filter === 'string' ? parseFilter(filter) : filter;
        applyFilter(qb, parsedFilter);
      }
      
      applyPagination(qb, { limit, order, page });
    });
    
    let includes: string[] = [];
    if (include) {
      includes = include.split(',');
      if (includes.includes('tags')) {
        query = query.fetch({ withRelated: ['tags'] });
      }
      if (includes.includes('authors')) {
        query = query.fetch({ withRelated: ['authors'] });
      }
      if (includes.includes('tiers')) {
        query = query.fetch({ withRelated: ['tiers'] });
      }
    }
    
    const posts = await query.fetchAll({ withRelated: includes || [] });
    
    return {
      success: true,
      data: posts.toJSON() as Post[],
      meta: {
        pagination: {
          page: page || 1,
          limit: limit || 15,
          pages: 1,
          total: posts.length,
          next: null,
          prev: null
        }
      }
    };
  }
  
  async function readPost(frame: any): Promise<ApiResponse<Post>> {
    const { id, slug, uuid } = frame.options;
    const include = frame.options.include;
    
    let post: any;
    
    if (id) {
      post = await PostModel.where({ id }).fetch({ withRelated: include?.split(',') || [] });
    } else if (slug) {
      post = await PostModel.where({ slug }).fetch({ withRelated: include?.split(',') || [] });
    } else if (uuid) {
      post = await PostModel.where({ uuid }).fetch({ withRelated: include?.split(',') || [] });
    }
    
    if (!post) {
      return {
        success: false,
        error: 'Post not found',
        status_code: 404
      };
    }
    
    return {
      success: true,
      data: post.toJSON() as Post
    };
  }
  
  async function addPost(data: Post, options: any): Promise<Post> {
    const post = PostModel.forge(data);
    const saved = await post.save(null, { method: 'insert' });
    
    if (saved.get('status') === 'published') {
      // Invalidate cache
    }
    
    return saved.toJSON() as Post;
  }
  
  async function editPost(frame: any, options: any): Promise<Post> {
    const { id, ...data } = frame.data.posts[0];
    const post = await PostModel.where({ id }).fetch({ require: true });
    
    const saved = await post.save(data, { method: 'update', patch: true });
    
    if (options.eventHandler) {
      options.eventHandler('published_updated', saved.toJSON() as Post);
    }
    
    return saved.toJSON() as Post;
  }
  
  async function bulkEdit(bulkData: any, options: any): Promise<any> {
    const { action, meta } = bulkData;
    const filter = options.filter;
    
    // Apply bulk action based on action type
    switch (action) {
      case 'published':
      case 'draft':
      case 'archived':
        await knexInstance('posts')
          .where((builder: any) => {
            if (filter) {
              const parsedFilter = parseFilter(filter);
              if (parsedFilter.status) {
                builder.where('status', parsedFilter.status);
              }
            }
          })
          .update({ status: action, updated_at: new Date().toISOString() });
        break;
    }
    
    return { updated: true };
  }
  
  async function bulkDestroy(options: any): Promise<any> {
    const filter = options.filter;
    
    await knexInstance('posts')
      .where((builder: any) => {
        if (filter) {
          const parsedFilter = parseFilter(filter);
          if (parsedFilter.status) {
            builder.where('status', parsedFilter.status);
          }
        }
      })
      .del();
    
    return { destroyed: true };
  }
  
  async function destroyPost(data: any, options: any): Promise<void> {
    const { id, require } = options;
    const post = await PostModel.where({ id }).fetch({ require });
    await post.destroy();
  }
  
  async function copyPost(frame: any): Promise<Post> {
    const { id } = frame.options;
    const original = await PostModel.where({ id }).fetch({ require: true, withRelated: ['tags', 'authors'] });
    const originalData = original.toJSON() as Post;
    
    // Create copy
    const copyData: Post = {
      ...originalData,
      id: '',
      uuid: '',
      slug: `${originalData.slug}-copy`,
      status: 'draft',
      created_at: new Date().toISOString(),
      updated_at: new Date().toISOString(),
      published_at: null
    };
    
    const copy = PostModel.forge(copyData);
    const saved = await copy.save(null, { method: 'insert' });
    
    // Copy relations
    if (originalData.tags) {
      const tagIds = (originalData.tags as any[]).map((t: any) => t.id);
      await saved.tags().attach(tagIds);
    }
    if (originalData.authors) {
      const authorIds = (originalData.authors as any[]).map((a: any) => a.id);
      await saved.authors().attach(authorIds);
    }
    
    return saved.toJSON() as Post;
  }
  
  async function exportPosts(options: any): Promise<any> {
    const { filter, limit, order } = options;
    
    let query = PostModel.forge().query((qb: any) => {
      qb.select('*');
      if (filter) {
        const parsedFilter = parseFilter(filter);
        applyFilter(qb, parsedFilter);
      }
      applyPagination(qb, { limit, order, page: 1 });
    });
    
    const posts = await query.fetchAll();
    
    // Convert to CSV
    const csv = posts.toJSON().map((post: Post) => 
      Object.values(post).map(v => `"${String(v).replace(/"/g, '""')}"`).join(',')
    ).join('\n');
    
    return { data: csv, filename: 'posts_export.csv' };
  }
  
  async function importCSV(data: any): Promise<any> {
    // Import logic would go here
    return { importId: 'import_123', total: 0 };
  }
  
  return {
    browsePosts,
    readPost,
    addPost,
    editPost,
    bulkEdit,
    bulkDestroy,
    destroyPost,
    copyPost,
    exportPosts,
    importCSV
  };
}

export const getPostServiceInstance = () => {
  // Singleton instance
  let instance: PostsService | null = null;
  
  return (knexInstance?: knex.Knex, bookshelfInstance?: any) => {
    if (!instance && knexInstance && bookshelfInstance) {
      instance = createPostsService(knexInstance, bookshelfInstance);
    }
    return instance!;
  };
};