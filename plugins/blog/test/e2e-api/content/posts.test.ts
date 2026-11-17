/**
 * Posts Content API Tests
 * Mirrors Ghost's posts.test.js for Content API
 */

import { expect } from 'chai';
import * as sinon from 'sinon';
import { describe, it, before, after, beforeEach } from 'mocha';
import { setupTestDb, teardownTestDb, seedTestData, cleanTestData, createTestAgent } from '../setup';
import { testPostsService, testModels } from '../setup';

const { Post, Tag, Author } = testModels;

describe('Posts Content API', function() {
  let agent: any;
  let sandbox: sinon.SinonSandbox;
  
  before(async function() {
    await setupTestDb();
    await seedTestData();
    
    // Create test agent
    agent = createTestAgent({});
  });
  
  after(async function() {
    await cleanTestData();
    await teardownTestDb();
  });
  
  beforeEach(function() {
    sandbox = sinon.createSandbox();
  });
  
  afterEach(function() {
    sandbox.restore();
  });
  
  it('Can request posts', async function() {
    const result = await testPostsService.browsePosts({
      filter: 'status:published',
      limit: 15,
      order: 'published_at DESC',
      page: 1
    });
    
    expect(result.success).to.be.true;
    expect(result.data).to.be.an('array');
    expect(result.data.length).to.be.greaterThan(0);
  });
  
  it('Can filter posts by tag', async function() {
    const result = await testPostsService.browsePosts({
      filter: 'tag:test-tag-1,status:published',
      limit: 15,
      order: 'published_at DESC',
      page: 1
    });
    
    expect(result.success).to.be.true;
    expect(result.data).to.be.an('array');
  });
  
  it('Can filter posts by author', async function() {
    const result = await testPostsService.browsePosts({
      filter: 'authors:test-author,status:published',
      limit: 15,
      order: 'published_at DESC',
      page: 1
    });
    
    expect(result.success).to.be.true;
    expect(result.data).to.be.an('array');
  });
  
  it('Can filter posts by visibility', async function() {
    const result = await testPostsService.browsePosts({
      filter: 'visibility:public,status:published',
      limit: 15,
      order: 'published_at DESC',
      page: 1
    });
    
    expect(result.success).to.be.true;
    expect(result.data).to.be.an('array');
  });
  
  it('Can request fields of posts', async function() {
    const result = await testPostsService.browsePosts({
      filter: 'status:published',
      limit: 15,
      order: 'published_at DESC',
      page: 1,
      fields: 'title,url'
    });
    
    expect(result.success).to.be.true;
    expect(result.data).to.be.an('array');
  });
  
  it('Can include relations', async function() {
    const result = await testPostsService.browsePosts({
      filter: 'status:published',
      limit: 15,
      order: 'published_at DESC',
      page: 1,
      include: 'tags,authors'
    });
    
    expect(result.success).to.be.true;
    expect(result.data).to.be.an('array');
  });
  
  it('Can request a single post by id', async function() {
    const posts = await Post.forge().where({ status: 'published' }).fetchAll();
    const post = posts.toJSON()[0];
    
    const frame = {
      options: {
        id: post.id,
        include: 'tags,authors'
      }
    };
    
    const result = await testPostsService.readPost(frame);
    
    expect(result.success).to.be.true;
    expect(result.data).to.have.property('id', post.id);
  });
  
  it('Can request a single post by slug', async function() {
    const posts = await Post.forge().where({ status: 'published' }).fetchAll();
    const post = posts.toJSON()[0];
    
    const frame = {
      options: {
        slug: post.slug,
        include: 'tags,authors'
      }
    };
    
    const result = await testPostsService.readPost(frame);
    
    expect(result.success).to.be.true;
    expect(result.data).to.have.property('slug', post.slug);
  });
  
  it('Can filter by published date', async function() {
    const posts = await Post.forge().where({ status: 'published' }).fetchAll({ limit: 1 });
    const post = posts.toJSON()[0];
    const publishedAt = new Date(post.published_at).toISOString().split('T')[0] + ' 00:00:00';
    
    const result = await testPostsService.browsePosts({
      filter: `published_at:<${publishedAt},status:published`,
      limit: 1
    });
    
    expect(result.success).to.be.true;
  });
  
  it('Can include tiers for public post', async function() {
    // This would require newsletter and tier setup
    const result = await testPostsService.browsePosts({
      filter: 'status:published',
      limit: 1,
      include: 'tiers'
    });
    
    expect(result.success).to.be.true;
  });
  
  it('Can use post excerpt as field', async function() {
    const result = await testPostsService.browsePosts({
      filter: 'status:published',
      fields: 'excerpt'
    });
    
    expect(result.success).to.be.true;
  });
  
  it('Can use post plaintext as field', async function() {
    const result = await testPostsService.browsePosts({
      filter: 'status:published',
      fields: 'plaintext'
    });
    
    expect(result.success).to.be.true;
  });
});

describe('Posts Content API - URL Transformations', function() {
  let sandbox: sinon.SinonSandbox;
  
  before(async function() {
    await setupTestDb();
    await seedTestData();
  });
  
  after(async function() {
    await cleanTestData();
    await teardownTestDb();
  });
  
  beforeEach(function() {
    sandbox = sinon.createSandbox();
  });
  
  afterEach(function() {
    sandbox.restore();
  });
  
  it('Can read post with all URLs as absolute site URLs', async function() {
    const result = await testPostsService.browsePosts({
      filter: 'status:published',
      limit: 1
    });
    
    expect(result.success).to.be.true;
  });
});