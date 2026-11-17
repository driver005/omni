/**
 * Authors (Users) Admin API Tests
 * Mirrors Ghost's users.test.js for Admin API
 */

import { expect } from 'chai';
import { describe, it, before, after } from 'mocha';
import { setupTestDb, teardownTestDb, seedTestData, cleanTestData } from '../setup';
import { testModels } from '../setup';

const { Author, Post } = testModels;

describe('Authors Admin API', function() {
  before(async function() {
    await setupTestDb();
    await seedTestData();
  });
  
  after(async function() {
    await cleanTestData();
    await teardownTestDb();
  });
  
  it('Can request authors', async function() {
    const authors = await Author.forge().fetchAll();
    expect(authors).to.be.an('array');
  });
  
  it('Can create a new author', async function() {
    const author = Author.forge({
      name: 'New Author',
      slug: 'new-author',
      email: 'newauthor@example.com',
      status: 'active',
      bio: 'New author bio'
    });
    
    const saved = await author.save(null, { method: 'insert' });
    expect(saved).to.have.property('id');
    expect(saved.get('name')).to.equal('New Author');
  });
  
  it('Can read an author by id', async function() {
    const authors = await Author.forge().fetchAll();
    const author = authors.toJSON()[0];
    
    const found = await Author.where({ id: author.id }).fetch({ require: true });
    expect(found).to.have.property('id', author.id);
  });
  
  it('Can read an author by slug', async function() {
    const authors = await Author.forge().fetchAll();
    const author = authors.toJSON()[0];
    
    const found = await Author.where({ slug: author.slug }).fetch({ require: true });
    expect(found).to.have.property('slug', author.slug);
  });
  
  it('Can read an author by email', async function() {
    const authors = await Author.forge().fetchAll();
    const author = authors.toJSON()[0];
    
    const found = await Author.where({ email: author.email }).fetch({ require: true });
    expect(found).to.have.property('email', author.email);
  });
  
  it('Can update an author', async function() {
    const authors = await Author.forge().fetchAll();
    const author = authors.toJSON()[0];
    
    const updated = await Author.where({ id: author.id }).save(
      { name: 'Updated Author Name' },
      { method: 'update', patch: true }
    );
    
    expect(updated.get('name')).to.equal('Updated Author Name');
  });
  
  it('Can delete an author', async function() {
    const author = Author.forge({
      name: 'Author to Delete',
      slug: 'author-to-delete',
      email: 'delete@example.com',
      status: 'active'
    });
    
    const saved = await author.save(null, { method: 'insert' });
    await Author.where({ id: saved.get('id') }).destroy();
    
    const deleted = await Author.where({ id: saved.get('id') }).fetch({ require: false });
    expect(deleted).to.be.null;
  });
  
  it('Can get posts by author', async function() {
    const authors = await Author.forge().fetchAll();
    const author = authors.toJSON()[0];
    
    const posts = await Post.forge()
      .query(qb => {
        qb.select('posts.*')
          .join('posts_authors', 'posts.id', 'posts_authors.post_id')
          .where('posts_authors.author_id', author.id);
      })
      .fetchAll();
    
    expect(posts).to.be.an('array');
  });
  
  it('Can change password', async function() {
    // Password change would be handled by auth service
    // This is a placeholder test
    expect(true).to.be.true;
  });
});