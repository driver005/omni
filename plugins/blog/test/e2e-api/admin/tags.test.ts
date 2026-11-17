/**
 * Tags Admin API Tests
 * Mirrors Ghost's tags.test.js for Admin API
 */

import { expect } from 'chai';
import { describe, it, before, after, beforeEach } from 'mocha';
import { setupTestDb, teardownTestDb, seedTestData, cleanTestData } from '../setup';
import { testModels } from '../setup';

const { Tag, Post } = testModels;

describe('Tags Admin API', function() {
  before(async function() {
    await setupTestDb();
    await seedTestData();
  });
  
  after(async function() {
    await cleanTestData();
    await teardownTestDb();
  });
  
  it('Can request tags', async function() {
    const tags = await Tag.forge().fetchAll();
    expect(tags).to.be.an('array');
  });
  
  it('Can create a new tag', async function() {
    const tag = Tag.forge({
      name: 'New Tag',
      slug: 'new-tag',
      description: 'A new tag'
    });
    
    const saved = await tag.save(null, { method: 'insert' });
    expect(saved).to.have.property('id');
    expect(saved.get('name')).to.equal('New Tag');
  });
  
  it('Can read a tag by id', async function() {
    const tags = await Tag.forge().fetchAll();
    const tag = tags.toJSON()[0];
    
    const found = await Tag.where({ id: tag.id }).fetch({ require: true });
    expect(found).to.have.property('id', tag.id);
  });
  
  it('Can read a tag by slug', async function() {
    const tags = await Tag.forge().fetchAll();
    const tag = tags.toJSON()[0];
    
    const found = await Tag.where({ slug: tag.slug }).fetch({ require: true });
    expect(found).to.have.property('slug', tag.slug);
  });
  
  it('Can update a tag', async function() {
    const tags = await Tag.forge().fetchAll();
    const tag = tags.toJSON()[0];
    
    const updated = await Tag.where({ id: tag.id }).save(
      { name: 'Updated Tag Name' },
      { method: 'update', patch: true }
    );
    
    expect(updated.get('name')).to.equal('Updated Tag Name');
  });
  
  it('Can delete a tag', async function() {
    const tag = Tag.forge({
      name: 'Tag to Delete',
      slug: 'tag-to-delete'
    });
    
    const saved = await tag.save(null, { method: 'insert' });
    await Tag.where({ id: saved.get('id') }).destroy();
    
    const deleted = await Tag.where({ id: saved.get('id') }).fetch({ require: false });
    expect(deleted).to.be.null;
  });
  
  it('Can get posts by tag', async function() {
    const tags = await Tag.forge().fetchAll();
    const tag = tags.toJSON()[0];
    
    const posts = await Post.forge()
      .query(qb => {
        qb.select('posts.*')
          .join('posts_tags', 'posts.id', 'posts_tags.post_id')
          .where('posts_tags.tag_id', tag.id);
      })
      .fetchAll();
    
    expect(posts).to.be.an('array');
  });
});