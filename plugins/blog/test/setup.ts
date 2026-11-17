/**
 * Test Setup for Ghost Blog Plugin
 * Configures test environment similar to Ghost's test framework
 */

import * as knex from 'knex';
import Bookshelf from 'bookshelf';
import { createKnex, createBookshelf, initModels, DatabaseConfig } from '../src/models';
import { createPostsService } from '../src/services/posts-service';

// Test database configuration
export const testDbConfig: DatabaseConfig = {
  client: 'sqlite3',
  connection: {
    filename: ':memory:'
  },
  pool: {
    min: 1,
    max: 1
  },
  migrations: {
    tableName: 'knex_migrations',
    directory: './migrations'
  }
};

// Global test instances
export let testKnex: any;
export let testBookshelf: any;
export let testModels: ReturnType<typeof initModels>;
export let testPostsService: ReturnType<typeof createPostsService>;

// Setup function
export async function setupTestDb() {
  testKnex = createKnex(testDbConfig);
  testBookshelf = createBookshelf(testKnex);
  testModels = initModels(testBookshelf);
  testPostsService = createPostsService(testKnex, testBookshelf);
  
  // Run migrations
  await testKnex.migrate.latest();
  
  return { testKnex, testBookshelf, testModels, testPostsService };
}

// Teardown function
export async function teardownTestDb() {
  if (testKnex) {
    await testKnex.destroy();
  }
}

// Seed test data
export async function seedTestData() {
  const { Post, Tag, Author } = testModels;
  
  // Create test author
  const author = await Author.forge({
    name: 'Test Author',
    slug: 'test-author',
    email: 'test@example.com',
    status: 'active',
    bio: 'Test author bio'
  }).save(null, { method: 'insert' });
  
  // Create test tags
  const tag1 = await Tag.forge({
    name: 'Test Tag 1',
    slug: 'test-tag-1',
    description: 'First test tag'
  }).save(null, { method: 'insert' });
  
  const tag2 = await Tag.forge({
    name: 'Test Tag 2',
    slug: 'test-tag-2',
    description: 'Second test tag'
  }).save(null, { method: 'insert' });
  
  // Create test posts
  const post1 = await Post.forge({
    title: 'Test Post 1',
    slug: 'test-post-1',
    content: '<p>Test content 1</p>',
    plaintext: 'Test content 1',
    author_id: author.get('id'),
    status: 'published',
    visibility: 'public',
    published_at: new Date().toISOString()
  }).save(null, { method: 'insert' });
  
  const post2 = await Post.forge({
    title: 'Test Post 2',
    slug: 'test-post-2',
    content: '<p>Test content 2</p>',
    plaintext: 'Test content 2',
    author_id: author.get('id'),
    status: 'draft',
    visibility: 'public'
  }).save(null, { method: 'insert' });
  
  // Associate tags with posts
  await post1.tags().attach([tag1.get('id'), tag2.get('id')]);
  await post2.tags().attach([tag1.get('id')]);
  
  // Associate authors with posts
  await post1.authors().attach([author.get('id')]);
  await post2.authors().attach([author.get('id')]);
  
  return { author, tags: [tag1, tag2], posts: [post1, post2] };
}

// Clean test data
export async function cleanTestData() {
  const { Post, Tag, Author, PostRevision } = testModels;
  
  await PostRevision.forge().query((qb: any) => qb.del()).fetch();
  await Post.forge().query((qb: any) => qb.del()).fetch();
  await Tag.forge().query((qb: any) => qb.del()).fetch();
  await Author.forge().query((qb: any) => qb.del()).fetch();
}

// Matchers for testing (mirroring Ghost's matchers)
export const matchers = {
  anyArray: (value: any) => Array.isArray(value),
  anyContentVersion: (value: any) => typeof value === 'string',
  anyEtag: (value: any) => typeof value === 'string' && value.startsWith('W/'),
  anyErrorId: (value: any) => typeof value === 'string',
  anyUuid: (value: any) => typeof value === 'string' && /^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/i.test(value),
  anyISODateTimeWithTZ: (value: any) => typeof value === 'string' && /^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(\.\d{3})?Z?$/.test(value)
};

// Helper to create test agent
export function createTestAgent(app: any) {
  return {
    get: (path: string) => ({
      expectStatus: (status: number) => ({
        matchHeaderSnapshot: (headers: any) => ({
          matchBodySnapshot: (body: any) => Promise.resolve({ body: {}, headers: {} })
        })
      }),
      header: (name: string, value: string) => createTestAgent(app).get(path),
      query: (params: any) => createTestAgent(app).get(path),
      authenticate: () => Promise.resolve()
    },
    post: (path: string) => ({
      send: (data: any) => ({
        expectStatus: (status: number) => ({
          matchBodySnapshot: (body: any) => Promise.resolve({ body: {} })
        })
      }),
      set: (header: string, value: string) => createTestAgent(app).post(path),
      authenticate: () => Promise.resolve()
    ),
    put: (path: string) => ({
      send: (data: any) => ({
        expectStatus: (status: number) => ({
          matchBodySnapshot: (body: any) => Promise.resolve({ body: {} })
        })
      }),
      authenticate: () => Promise.resolve()
    }),
    delete: (path: string) => ({
      expectStatus: (status: number) => ({
        matchBodySnapshot: (body: any) => Promise.resolve({ body: {} })
      }),
      authenticate: () => Promise.resolve()
    })
  };
  }
}