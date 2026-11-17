/**
 * Posts Admin API Tests
 * Mirrors Ghost's posts.test.js for Admin API
 */

import { expect } from 'chai';
import * as sinon from 'sinon';
import { describe, it, before, after, beforeEach } from 'mocha';
import { setupTestDb, teardownTestDb, seedTestData, cleanTestData } from '../setup';
import { testPostsService, testModels } from '../setup';

const { Post, Tag, Author } = testModels;

describe('Posts Admin API', function() {
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
  
  describe('Browse Posts', function() {
    it('Can request posts', async function() {
      const result = await testPostsService.browsePosts({
        filter: 'status:all',
        limit: 15,
        order: 'published_at DESC',
        page: 1
      });
      
      expect(result.success).to.be.true;
      expect(result.data).to.be.an('array');
    });
    
    it('Can filter posts by status', async function() {
      const result = await testPostsService.browsePosts({
        filter: 'status:draft',
        limit: 15,
        order: 'published_at DESC',
        page: 1
      });
      
      expect(result.success).to.be.true;
    });
    
    it('Can filter posts by tag', async function() {
      const result = await testPostsService.browsePosts({
        filter: 'tag:test-tag-1,status:all',
        limit: 15,
        order: 'published_at DESC',
        page: 1
      });
      
      expect(result.success).to.be.true;
    });
    
    it('Can filter posts by author', async function() {
      const result = await testPostsService.browsePosts({
        filter: 'authors:test-author,status:all',
        limit: 15,
        order: 'published_at DESC',
        page: 1
      });
      
      expect(result.success).to.be.true;
    });
    
    it('Can request fields of posts', async function() {
      const result = await testPostsService.browsePosts({
        filter: 'status:all',
        fields: 'title,slug',
        limit: 15
      });
      
      expect(result.success).to.be.true;
    });
    
    it('Can include relations', async function() {
      const result = await testPostsService.browsePosts({
        filter: 'status:all',
        include: 'tags,authors',
        limit: 15
      });
      
      expect(result.success).to.be.true;
    });
  });
  
  describe('Read Post', function() {
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
  });
  
  describe('Add Post', function() {
    it('Can create a new post', async function() {
      const author = await Author.forge().where({ slug: 'test-author' }).fetch();
      
      const postData = {
        title: 'New Test Post',
        slug: 'new-test-post',
        content: '<p>New test content</p>',
        plaintext: 'New test content',
        author_id: author.get('id'),
        status: 'draft',
        visibility: 'public'
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
    
    it('Can create a published post', async function() {
      const author = await Author.forge().where({ slug: 'test-author' }).fetch();
      
      const postData = {
        title: 'Published Test Post',
        slug: 'published-test-post',
        content: '<p>Published test content</p>',
        plaintext: 'Published test content',
        author_id: author.get('id'),
        status: 'published',
        visibility: 'public',
        published_at: new Date().toISOString()
      };
      
      const post = await testPostsService.addPost(postData, {
        include: 'tags,authors',
        formats: 'html',
        source: 'html'
      });
      
      expect(post).to.have.property('id');
      expect(post.status).to.equal('published');
      expect(post.published_at).to.not.be.empty;
    });
  });
  
  describe('Edit Post', function() {
    it('Can update a post', async function() {
      const posts = await Post.forge().where({ status: 'draft' }).fetchAll();
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
  
  describe('Delete Post', function() {
    it('Can delete a post', async function() {
      const posts = await Post.forge().where({ status: 'draft' }).fetchAll();
      const post = posts.toJSON()[0];
      
      await testPostsService.destroyPost({}, { id: post.id, require: true });
      
      const deleted = await Post.where({ id: post.id }).fetch({ require: false });
      expect(deleted).to.be.null;
    });
  });
  
  describe('Bulk Operations', function() {
    it('Can bulk destroy posts', async function() {
      const result = await testPostsService.bulkDestroy({
        filter: 'status:draft'
      });
      
      expect(result).to.have.property('destroyed', true);
    });
    
    it('Can bulk edit posts', async function() {
      const result = await testPostsService.bulkEdit(
        { action: 'published', meta: {} },
        { filter: 'status:draft' }
      );
      
      expect(result).to.have.property('updated', true);
    });
  });
  
  describe('Copy Post', function() {
    it('Can copy a post', async function() {
      const posts = await Post.forge().where({ status: 'published' }).fetchAll();
      const post = posts.toJSON()[0];
      
      const frame = {
        options: {
          id: post.id,
          formats: 'html'
        }
      };
      
      const copy = await testPostsService.copyPost(frame);
      
      expect(copy).to.have.property('id');
      expect(copy.slug).to.equal(`${post.slug}-copy`);
      expect(copy.status).to.equal('draft');
    });
  });
  
  describe('Export/Import', function() {
    it('Can export posts to CSV', async function() {
      const result = await testPostsService.exportPosts({
        filter: 'status:all',
        limit: 15,
        order: 'published_at DESC'
      });
      
      expect(result).to.have.property('data');
      expect(result).to.have.property('filename', 'posts_export.csv');
    });
    
    it('Can import posts from CSV', async function() {
      const result = await testPostsService.importCSV({
        filePath: 'test.csv',
        fileName: 'test.csv',
        mapping: {},
        requestUserEmail: 'test@example.com'
      });
      
      expect(result).to.have.property('importId');
      expect(result).to.have.property('total');
    });
  });
});