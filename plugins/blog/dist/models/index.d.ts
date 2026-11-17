/**
 * Ghost Blog Plugin Models
 * Mirrors Ghost's Bookshelf models for API compatibility
 */
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
export declare function createKnex(config: DatabaseConfig): any;
export declare function createBookshelf(knexInstance: any): any;
export declare function initModels(bookshelfInstance: any): {
    Post: any;
    Tag: any;
    Author: any;
    Newsletter: any;
    Tier: any;
    Member: any;
    PostRevision: any;
    Settings: any;
};
//# sourceMappingURL=index.d.ts.map