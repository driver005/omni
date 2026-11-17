module;
#include <gtest/gtest.h>
#include <memory>

export module blog_api:test;

import std;
import blog_api:types;
import blog_database:database;
import blog_database:post_repository;
import blog_database:tag_repository;
import blog_database:author_repository;

using namespace ghost::blog;
using namespace ghost::blog::database;

class BlogApiTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test database (SQLite in-memory)
        auto db_config = DatabaseConfig::create_sqlite(":memory:");
        if (db_config) {
            m_database = std::move(*db_config);
            m_database->connect("sqlite::memory:");
            m_database->create_schema();
        }
        
        m_post_repo = std::make_unique<PostRepository>(m_database);
        m_tag_repo = std::make_unique<TagRepository>(m_database);
        m_author_repo = std::make_unique<AuthorRepository>(m_database);
    }
    
    void TearDown() override {
        if (m_database) {
            m_database->disconnect();
        }
    }
    
    std::shared_ptr<IDatabase> m_database;
    std::unique_ptr<PostRepository> m_post_repo;
    std::unique_ptr<TagRepository> m_tag_repo;
    std::unique_ptr<AuthorRepository> m_author_repo;
};

TEST_F(BlogApiTest, CreatePost) {
    Post post = Post::create_new("Test Post", "author-123");
    post.set_content("<p>Test content</p>");
    post.set_plaintext("Test content");
    
    auto result = m_post_repo->create(post);
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->get_success());
    ASSERT_TRUE(result->get_data().has_value());
    
    auto created_post = *result->get_data();
    ASSERT_EQ(created_post.get_title(), "Test Post");
    ASSERT_EQ(created_post.get_author_id(), "author-123");
    ASSERT_EQ(created_post.get_status(), "draft");
}

TEST_F(BlogApiTest, FindPostById) {
    Post post = Post::create_new("Test Post", "author-123");
    auto create_result = m_post_repo->create(post);
    ASSERT_TRUE(create_result);
    
    auto created_post = *create_result->get_data();
    auto find_result = m_post_repo->find_by_id(created_post.get_id());
    ASSERT_TRUE(find_result);
    ASSERT_TRUE(find_result->get_data().has_value());
    
    auto found_post = *find_result->get_data();
    ASSERT_EQ(found_post.get_id(), created_post.get_id());
    ASSERT_EQ(found_post.get_title(), "Test Post");
}

TEST_F(BlogApiTest, FindPostBySlug) {
    Post post = Post::create_new("Test Post", "author-123");
    auto create_result = m_post_repo->create(post);
    ASSERT_TRUE(create_result);
    
    auto created_post = *create_result->get_data();
    auto find_result = m_post_repo->find_by_slug(created_post.get_slug());
    ASSERT_TRUE(find_result);
    ASSERT_TRUE(find_result->get_data().has_value());
    
    auto found_post = *find_result->get_data();
    ASSERT_EQ(found_post.get_slug(), created_post.get_slug());
}

TEST_F(BlogApiTest, UpdatePost) {
    Post post = Post::create_new("Test Post", "author-123");
    auto create_result = m_post_repo->create(post);
    ASSERT_TRUE(create_result);
    
    auto created_post = *create_result->get_data();
    created_post.set_title("Updated Title");
    created_post.set_content("<p>Updated content</p>");
    
    auto update_result = m_post_repo->update(created_post.get_id(), created_post);
    ASSERT_TRUE(update_result);
    ASSERT_TRUE(update_result->get_success());
    
    auto updated_post = *update_result->get_data();
    ASSERT_EQ(updated_post.get_title(), "Updated Title");
}

TEST_F(BlogApiTest, DeletePost) {
    Post post = Post::create_new("Test Post", "author-123");
    auto create_result = m_post_repo->create(post);
    ASSERT_TRUE(create_result);
    
    auto created_post = *create_result->get_data();
    auto delete_result = m_post_repo->delete_by_id(created_post.get_id());
    ASSERT_TRUE(delete_result);
    ASSERT_TRUE(delete_result->get_success());
    ASSERT_TRUE(*delete_result->get_data());
    
    // Verify post is deleted
    auto find_result = m_post_repo->find_by_id(created_post.get_id());
    ASSERT_TRUE(find_result);
    ASSERT_FALSE(find_result->get_data().has_value());
}

TEST_F(BlogApiTest, PublishPost) {
    Post post = Post::create_new("Test Post", "author-123");
    auto create_result = m_post_repo->create(post);
    ASSERT_TRUE(create_result);
    
    auto created_post = *create_result->get_data();
    auto publish_result = m_post_repo->publish(created_post.get_id());
    ASSERT_TRUE(publish_result);
    ASSERT_TRUE(publish_result->get_success());
    
    auto published_post = *publish_result->get_data();
    ASSERT_EQ(published_post.get_status(), "published");
    ASSERT_FALSE(published_post.get_published_at().empty());
}

TEST_F(BlogApiTest, CreateTag) {
    Tag tag;
    tag.set_id(Tag::generate_uuid());
    tag.set_name("Test Tag");
    tag.set_slug("test-tag");
    tag.set_description("A test tag");
    
    auto result = m_tag_repo->create(tag);
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->get_success());
    ASSERT_TRUE(result->get_data().has_value());
    
    auto created_tag = *result->get_data();
    ASSERT_EQ(created_tag.get_name(), "Test Tag");
    ASSERT_EQ(created_tag.get_slug(), "test-tag");
}

TEST_F(BlogApiTest, CreateAuthor) {
    Author author;
    author.set_id(Author::generate_uuid());
    author.set_name("Test Author");
    author.set_slug("test-author");
    author.set_email("test@example.com");
    author.set_bio("A test author");
    
    auto result = m_author_repo->create(author);
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->get_success());
    ASSERT_TRUE(result->get_data().has_value());
    
    auto created_author = *result->get_data();
    ASSERT_EQ(created_author.get_name(), "Test Author");
    ASSERT_EQ(created_author.get_email(), "test@example.com");
}

TEST_F(BlogApiTest, PostTagRelationship) {
    // Create a post
    Post post = Post::create_new("Test Post", "author-123");
    auto create_result = m_post_repo->create(post);
    ASSERT_TRUE(create_result);
    auto created_post = *create_result->get_data();
    
    // Create a tag
    Tag tag;
    tag.set_id(Tag::generate_uuid());
    tag.set_name("Test Tag");
    tag.set_slug("test-tag");
    auto tag_result = m_tag_repo->create(tag);
    ASSERT_TRUE(tag_result);
    auto created_tag = *tag_result->get_data();
    
    // Add tag to post
    auto add_result = m_post_repo->add_tag(created_post.get_id(), created_tag.get_id());
    ASSERT_TRUE(add_result);
    ASSERT_TRUE(add_result->get_success());
    
    // Get tags for post
    auto tags_result = m_post_repo->get_tags(created_post.get_id());
    ASSERT_TRUE(tags_result);
    ASSERT_TRUE(tags_result->get_success());
    
    // Remove tag from post
    auto remove_result = m_post_repo->remove_tag(created_post.get_id(), created_tag.get_id());
    ASSERT_TRUE(remove_result);
    ASSERT_TRUE(remove_result->get_success());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}