/**
 * Posts API Tests for Ghost Blog Plugin
 */

import { expect } from 'chai';
import { describe, it, before, after } from 'mocha';
import { setupTestDb, teardownTestDb, seedTestData, cleanTestData } from '../../setup';
import { testPostsService, testModels } from '../../setup';

const { Post, Tag, Author } = testModels;

describe('Posts API', function() {
  before(async function() {
    await setupTestDb();
    await seedTestData();
  });
  
  after(async function() {
    await cleanTestData();
    await teardownTestDb();
  });
  
  it('Can browse posts', async function() {
    const result = await testPostsService.browsePosts({
      filter: 'status:published',
      limit: 15,
      order: 'published_at DESC',
      page: 1
    });
    
    expect(result.success).to.be.true;
    expect(result.data).to.be.an('array');
    expect(result.data!.length).to.be.greaterThan(0);
  });
  
  it('Can read a single post by id', async function() {
    const posts = await testModels.Post.forge().where({ status: 'published' }).fetchAll();
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
  
  it('Can create a new post', async function() {
    const author = await testModels.Author.forge().where({ slug: 'test-author' }).fetch();
    
    const now = new Date().toISOString();
    const postData = {
      id: '',
      uuid: '',
      title: 'New Test Post',
      slug: 'new-test-post',
      content: '<p>New test content</p>',
      plaintext: 'New test content',
      author_id: author.get('id'),
      status: 'draft' as const,
      visibility: 'public' as const,
      created_at: now,
      updated_at: now,
      published_at: null,
      scheduled_at: null,
      feature_image: null,
      feature_image_alt: null,
      feature_image_caption: null,
      excerpt: null,
      codeinjection_head: null,
      codeinjection_foot: null,
      custom_excerpt: null,
      canonical_url: null,
      meta_title: null,
      meta_description: null,
      og_image: null,
      og_title: null,
      og_description: null,
      twitter_image: null,
      twitter_title: null,
      twitter_description: null,
      custom_template: null,
      email_only: null,
      comment_id: null,
      meta: {},
      tags: [],
      authors: []
    };
    
    const post = await testPostsService.addPost(postData, {
      include: 'tags,authors',
      formats: 'html',
      source: 'html'
    });
    
    expect(post).to.have.property('id');
    expect(post.title).to.equal('New Test Post');
    expect(post.status).to.equal('draft');
  });
  
  it('Can update a post', async function() {
    const posts = await testModels.Post.forge().where({ status: 'draft' }).fetchAll();
    const post = posts.toJSON()[0];
    
    const frame = {
      data: { posts: [{ ...post, title: 'Updated Title' }] },
      options: {
        id: post.id,
        include: 'tags,authors',
        formats: 'html',
        source: 'html'
      }
    };
    
    const updated = await testPostsService.editPost(frame, {
      eventHandler: (event, dto) => {}
    });
    
    expect(updated.title).to.equal('Updated Title');
  });
});