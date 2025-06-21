#include <gtest/gtest.h>
#include "FileReader.h"
#include "ErrorHandler.h"
#include <fstream>
#include <filesystem>

using namespace yaml2json;

class FileReaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test files
        std::ofstream test_file("test_file.txt");
        test_file << "Hello, World!";
        test_file.close();
        
        std::ofstream empty_file("empty_file.txt");
        empty_file.close();
    }
    
    void TearDown() override {
        // Clean up test files
        std::filesystem::remove("test_file.txt");
        std::filesystem::remove("empty_file.txt");
    }
};

TEST_F(FileReaderTest, ReadFile_Success) {
    FileContent content = FileReader::read_file("test_file.txt");
    
    ASSERT_TRUE(content.is_valid());
    EXPECT_EQ(content.size(), 13);
    EXPECT_EQ(std::string(content.data(), content.size()), "Hello, World!");
}

TEST_F(FileReaderTest, ReadFile_NonExistentFile) {
    EXPECT_THROW({
        FileReader::read_file("nonexistent.txt");
    }, ConversionError);
}

TEST_F(FileReaderTest, ReadFile_EmptyFile) {
    EXPECT_THROW({
        FileReader::read_file("empty_file.txt");
    }, ConversionError);
}

TEST_F(FileReaderTest, ValidateFile_ExistingFile) {
    EXPECT_NO_THROW({
        FileReader::validate_file("test_file.txt");
    });
}

TEST_F(FileReaderTest, ValidateFile_NonExistentFile) {
    EXPECT_THROW({
        FileReader::validate_file("nonexistent.txt");
    }, ConversionError);
}

TEST_F(FileReaderTest, ValidateFile_Directory) {
    std::filesystem::create_directory("test_dir");
    
    EXPECT_THROW({
        FileReader::validate_file("test_dir");
    }, ConversionError);
    
    std::filesystem::remove("test_dir");
}

TEST_F(FileReaderTest, FileContent_MoveSemantics) {
    FileContent content1 = FileReader::read_file("test_file.txt");
    ASSERT_TRUE(content1.is_valid());
    
    FileContent content2 = std::move(content1);
    EXPECT_TRUE(content2.is_valid());
    EXPECT_EQ(content2.size(), 13);
    
    // After move, content1 should be in a valid but unspecified state
    // We don't test content1 after move as its state is implementation-defined
}