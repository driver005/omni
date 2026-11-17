/**
 * Ghost Blog Plugin Types
 * Mirrors Ghost's core types for API compatibility
 */
export interface PaginationParams {
    page?: number;
    limit?: number;
    order?: string;
    direction?: 'ASC' | 'DESC';
}
export interface PostFilter {
    status?: string;
    author_id?: string;
    tag?: string;
    search?: string;
    visibility?: string;
}
export interface Post {
    id: string;
    uuid: string;
    title: string;
    slug: string;
    content: string;
    plaintext: string;
    author_id: string;
    status: 'draft' | 'published' | 'scheduled' | 'archived';
    visibility: 'public' | 'members' | 'paid' | 'tiers';
    created_at: string;
    updated_at: string;
    published_at: string | null;
    scheduled_at: string | null;
    feature_image: string | null;
    feature_image_alt: string | null;
    feature_image_caption: string | null;
    excerpt: string | null;
    codeinjection_head: string | null;
    codeinjection_foot: string | null;
    custom_excerpt: string | null;
    canonical_url: string | null;
    meta_title: string | null;
    meta_description: string | null;
    og_image: string | null;
    og_title: string | null;
    og_description: string | null;
    twitter_image: string | null;
    twitter_title: string | null;
    twitter_description: string | null;
    custom_template: string | null;
    email_only: string | null;
    comment_id: string | null;
    meta: Record<string, unknown>;
    tags: string[];
    authors: string[];
    url?: string;
    html?: string;
    access?: boolean;
    primary_author?: Author;
    tiers?: Tier[];
    newsletter?: Newsletter;
}
export interface Tag {
    id: string;
    name: string;
    slug: string;
    description: string | null;
    feature_image: string | null;
    visibility: 'public' | 'internal';
    meta_title: string | null;
    meta_description: string | null;
    og_image: string | null;
    og_title: string | null;
    og_description: string | null;
    twitter_image: string | null;
    twitter_title: string | null;
    twitter_description: string | null;
    created_at: string;
    updated_at: string;
    post_count?: number;
    url?: string;
}
export interface Author {
    id: string;
    name: string;
    slug: string;
    email: string;
    profile_image: string | null;
    cover_image: string | null;
    bio: string | null;
    website: string | null;
    location: string | null;
    accessibility: string | null;
    status: 'active' | 'inactive' | 'invited';
    last_login: string | null;
    created_at: string;
    updated_at: string;
    meta: Record<string, unknown>;
    post_count?: number;
    url?: string;
}
export interface Newsletter {
    id: string;
    uuid: string;
    name: string;
    slug: string;
    description: string | null;
    sender_name: string | null;
    sender_email: string | null;
    sender_reply_to: string | null;
    title: string | null;
    custom_excerpt: string | null;
    show_title_and_feature_image: boolean;
    locale: string;
    status: 'active' | 'inactive';
    visibility: 'public' | 'members' | 'paid' | 'tiers';
    feature_image: string | null;
    feature_image_alt: string | null;
    feature_image_caption: string | null;
    meta_title: string | null;
    meta_description: string | null;
    og_image: string | null;
    og_title: string | null;
    og_description: string | null;
    twitter_image: string | null;
    twitter_title: string | null;
    twitter_description: string | null;
    custom_template: string | null;
    email_only: string | null;
    email_recipient_filter: string | null;
    created_at: string;
    updated_at: string;
    meta: Record<string, unknown>;
    tiers: string[];
    segments: string[];
}
export interface Tier {
    id: string;
    uuid: string;
    name: string;
    slug: string;
    description: string | null;
    type: 'free' | 'paid';
    active: boolean;
    visibility: 'public' | 'none';
    monthly_price: number;
    yearly_price: number;
    trial_days: number;
    currency: string;
    welcome_page_url: string | null;
    created_at: string;
    updated_at: string;
    meta: Record<string, unknown>;
    benefits: Array<{
        type: string;
        name: string;
        icons: string[];
        description: string;
    }>;
}
export interface Member {
    id: string;
    uuid: string;
    email: string;
    name: string | null;
    note: string | null;
    avatar_image: string | null;
    status: 'free' | 'paid' | 'comp' | 'none' | 'cancelled';
    visibility: 'public' | 'none';
    comp_email: string | null;
    comp_source: string | null;
    subscribed: boolean;
    created_at: string;
    updated_at: string;
    last_login: string | null;
    last_seen: string | null;
    last_signin: string | null;
    logins_count: number;
    meta: Record<string, unknown>;
    labels: string[];
    subscriptions: Array<{
        id: string;
        name: string;
        status: string;
        start_date: string;
        end_date: string | null;
    }>;
}
export interface ApiResponse<T> {
    success: boolean;
    data?: T;
    meta?: Record<string, unknown>;
    error?: string;
    status_code?: number;
}
export interface Settings {
    [key: string]: {
        value: unknown;
        type: 'string' | 'boolean' | 'number' | 'json';
        group: string;
        description: string;
        public: boolean;
    };
}
export interface Site {
    id: string;
    uuid: string;
    title: string;
    description: string | null;
    logo: string | null;
    icon: string | null;
    cover_image: string | null;
    facebook: string | null;
    twitter: string | null;
    url: string;
    version: string;
    timezone: string;
    locale: string;
    language: string;
    ghost_version: string;
    environment: string;
    meta: Record<string, unknown>;
}
//# sourceMappingURL=index.d.ts.map