/**
 * Ghost Blog Plugin Models
 * Mirrors Ghost's Bookshelf models for API compatibility
 */

const knex = require('knex');
const Bookshelf = require('bookshelf');
import { Post, Tag, Author, Newsletter, Tier, Member, Settings, Site } from '../types';

export interface DatabaseConfig {
  client: 'pg' | 'sqlite3';
  connection: {
    host?: string;
    port?: number;
    database?: string;
    user?: string;
    password?: string;
    filename?: string;
  };
  pool?: {
    min: number;
    max: number;
  };
  migrations?: {
    tableName: string;
    directory: string;
  };
  seeds?: {
    directory: string;
  };
}

export function createKnex(config: DatabaseConfig): any {
  return knex(config);
}

export function createBookshelf(knexInstance: any): any {
  const Bookshelf = require('bookshelf');
  const bookshelfInstance = Bookshelf(knexInstance);
  
  // Register plugins
  bookshelfInstance.plugin('registry');
  bookshelfInstance.plugin('visibility');
  
  return bookshelfInstance;
}

// Base Model with common functionality
const BaseModel = (bookshelfInstance: any) => {
  return bookshelfInstance.Model.extend({
    hasTimestamps: ['created_at', 'updated_at'],
    
    // Common virtual attributes
    virtuals: {
      url: function(this: any) {
        return this.getUrl();
      }
    },
    
    getUrl(): string {
      return '';
    },
    
    // Serialization
    toJSON(options?: { visible?: string[]; hidden?: string[] }): Record<string, unknown> {
      const json = super.toJSON(options);
      return json;
    }
  });
};

// Post Model
function createPostModel(bookshelfInstance: any) {
  const Base = BaseModel(bookshelfInstance);
  
  return Base.extend({
    tableName: 'posts',
    idAttribute: 'id',
    
    allowedFormats: ['html', 'plaintext', 'lexical', 'mobiledoc'],
    
    tags() {
      return this.belongsToMany('Tag', 'posts_tags', 'post_id', 'tag_id');
    },
    
    authors() {
      return this.belongsToMany('Author', 'posts_authors', 'post_id', 'author_id');
    },
    
    primaryAuthor() {
      return this.belongsTo('Author', 'author_id');
    },
    
    newsletter() {
      return this.belongsTo('Newsletter', 'newsletter_id');
    },
    
    tiers() {
      return this.belongsToMany('Tier', 'posts_tiers', 'post_id', 'tier_id');
    },
    
    revisions() {
      return this.hasMany('PostRevision', 'post_id');
    },
    
    getUrl(): string {
      const siteUrl = process.env.GHOST_URL || 'http://localhost:2368';
      return `${siteUrl}/${this.get('slug')}/`;
    }
  });
}

// Tag Model
function createTagModel(bookshelfInstance: any) {
  const Base = BaseModel(bookshelfInstance);
  
  return Base.extend({
    tableName: 'tags',
    idAttribute: 'id',
    
    posts() {
      return this.belongsToMany('Post', 'posts_tags', 'tag_id', 'post_id');
    },
    
    getUrl(): string {
      const siteUrl = process.env.GHOST_URL || 'http://localhost:2368';
      return `${siteUrl}/tag/${this.get('slug')}/`;
    }
  });
}

// Author Model
function createAuthorModel(bookshelfInstance: any) {
  const Base = BaseModel(bookshelfInstance);
  
  return Base.extend({
    tableName: 'users',
    idAttribute: 'id',
    
    posts() {
      return this.belongsToMany('Post', 'posts_authors', 'author_id', 'post_id');
    },
    
    getUrl(): string {
      const siteUrl = process.env.GHOST_URL || 'http://localhost:2368';
      return `${siteUrl}/author/${this.get('slug')}/`;
    }
  });
}

// Newsletter Model
function createNewsletterModel(bookshelfInstance: any) {
  const Base = BaseModel(bookshelfInstance);
  
  return Base.extend({
    tableName: 'newsletters',
    idAttribute: 'id',
    
    posts() {
      return this.hasMany('Post', 'newsletter_id');
    },
    
    tiers() {
      return this.belongsToMany('Tier', 'newsletters_tiers', 'newsletter_id', 'tier_id');
    }
  });
}

// Tier Model
function createTierModel(bookshelfInstance: any) {
  const Base = BaseModel(bookshelfInstance);
  
  return Base.extend({
    tableName: 'tiers',
    idAttribute: 'id',
    
    posts() {
      return this.belongsToMany('Post', 'posts_tiers', 'tier_id', 'post_id');
    },
    
    newsletters() {
      return this.belongsToMany('Newsletter', 'newsletters_tiers', 'tier_id', 'newsletter_id');
    },
    
    benefits() {
      return this.hasMany('TierBenefit', 'tier_id');
    }
  });
}

// Member Model
function createMemberModel(bookshelfInstance: any) {
  const Base = BaseModel(bookshelfInstance);
  
  return Base.extend({
    tableName: 'members',
    idAttribute: 'id',
    
    subscriptions() {
      return this.hasMany('MemberSubscription', 'member_id');
    },
    
    labels() {
      return this.belongsToMany('Label', 'members_labels', 'member_id', 'label_id');
    },
    
    stripeCustomers() {
      return this.hasMany('MemberStripeCustomer', 'member_id');
    },
    
    loginEvents() {
      return this.hasMany('MemberLoginEvent', 'member_id');
    }
  });
}

// PostRevision Model
function createPostRevisionModel(bookshelfInstance: any) {
  const Base = BaseModel(bookshelfInstance);
  
  return Base.extend({
    tableName: 'posts_meta',
    idAttribute: 'id',
    
    post() {
      return this.belongsTo('Post', 'post_id');
    }
  });
}

// Settings Model
function createSettingsModel(bookshelfInstance: any) {
  const Base = BaseModel(bookshelfInstance);
  
  return Base.extend({
    tableName: 'settings',
    idAttribute: 'key',
    
    hasTimestamps: false
  });
}

// Initialize all models
export function initModels(bookshelfInstance: any) {
  const Post = createPostModel(bookshelfInstance);
  const Tag = createTagModel(bookshelfInstance);
  const Author = createAuthorModel(bookshelfInstance);
  const Newsletter = createNewsletterModel(bookshelfInstance);
  const Tier = createTierModel(bookshelfInstance);
  const Member = createMemberModel(bookshelfInstance);
  const PostRevision = createPostRevisionModel(bookshelfInstance);
  const Settings = createSettingsModel(bookshelfInstance);
  
  // Register models in registry
  bookshelfInstance.model('Post', Post);
  bookshelfInstance.model('Tag', Tag);
  bookshelfInstance.model('Author', Author);
  bookshelfInstance.model('Newsletter', Newsletter);
  bookshelfInstance.model('Tier', Tier);
  bookshelfInstance.model('Member', Member);
  bookshelfInstance.model('PostRevision', PostRevision);
  bookshelfInstance.model('Settings', Settings);
  
  return {
    Post,
    Tag,
    Author,
    Newsletter,
    Tier,
    Member,
    PostRevision,
    Settings
  };
}

// Remove the duplicate export at the bottom