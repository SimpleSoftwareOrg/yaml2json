#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "FileReader.h"
#include "YamlToJsonConverter.h"
#include "JsonFormatter.h"
#include "ErrorHandler.h"

using namespace yaml2json;

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        setup_error_handlers();
    }
    
    std::string readFile(const std::string& path) {
        std::ifstream file(path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};

TEST_F(IntegrationTest, ConvertSimpleYaml) {
    // Full workflow: read YAML file -> convert -> format
    auto content = FileReader::read_file("tests/test_data/simple.yaml");
    std::string json = YamlToJsonConverter::convert(content.data(), content.size(), "simple.yaml");
    
    // Verify the conversion
    EXPECT_NE(json.find(R"("name": "test")"), std::string::npos);
    EXPECT_NE(json.find(R"("version": 1.0)"), std::string::npos);
    EXPECT_NE(json.find(R"("enabled": true)"), std::string::npos);
    EXPECT_NE(json.find(R"("count": 42)"), std::string::npos);
    EXPECT_NE(json.find(R"("price": 3.14)"), std::string::npos);
}

TEST_F(IntegrationTest, ConvertNestedYaml) {
    auto content = FileReader::read_file("tests/test_data/nested.yaml");
    std::string json = YamlToJsonConverter::convert(content.data(), content.size(), "nested.yaml");
    
    // Verify nested structure
    EXPECT_NE(json.find(R"("project": {)"), std::string::npos);
    EXPECT_NE(json.find(R"("name": "yaml2json")"), std::string::npos);
    EXPECT_NE(json.find(R"("features": [)"), std::string::npos);
    EXPECT_NE(json.find(R"("dependencies": {)"), std::string::npos);
}

TEST_F(IntegrationTest, ConvertAndPrettyPrint) {
    auto content = FileReader::read_file("tests/test_data/simple.yaml");
    std::string json = YamlToJsonConverter::convert(content.data(), content.size());
    std::string pretty = JsonFormatter::pretty_print(json);
    
    // Verify pretty printing adds newlines and indentation
    EXPECT_GT(pretty.length(), json.length());
    EXPECT_NE(pretty.find('\n'), std::string::npos);
    EXPECT_NE(pretty.find("  "), std::string::npos);
}

TEST_F(IntegrationTest, HandleInvalidYaml) {
    EXPECT_THROW({
        auto content = FileReader::read_file("tests/test_data/invalid.yaml");
        YamlToJsonConverter::convert(content.data(), content.size(), "invalid.yaml");
    }, ConversionError);
}

TEST_F(IntegrationTest, HandleEmptyFile) {
    // Create an empty file
    std::ofstream empty("empty_test.yaml");
    empty.close();
    
    EXPECT_THROW({
        FileReader::read_file("empty_test.yaml");
    }, ConversionError);
    
    std::filesystem::remove("empty_test.yaml");
}

TEST_F(IntegrationTest, LargeFile) {
    // Create a large YAML file
    std::ofstream large("large_test.yaml");
    large << "items:\n";
    for (int i = 0; i < 1000; ++i) {
        large << "  - item" << i << "\n";
    }
    large.close();
    
    // Should handle large files efficiently
    EXPECT_NO_THROW({
        auto content = FileReader::read_file("large_test.yaml");
        std::string json = YamlToJsonConverter::convert(content.data(), content.size());
        EXPECT_GT(json.length(), 5000); // Should produce a large JSON
    });
    
    std::filesystem::remove("large_test.yaml");
}

TEST_F(IntegrationTest, SpecialCharactersInYaml) {
    // Create YAML with special characters
    std::ofstream special("special_test.yaml");
    special << R"(
text: "Hello \"World\""
multiline: |
  Line 1
  Line 2
unicode: "Hello 世界"
special_chars: "Tab\there and newline\nhere"
)";
    special.close();
    
    auto content = FileReader::read_file("special_test.yaml");
    std::string json = YamlToJsonConverter::convert(content.data(), content.size());
    
    // Verify special characters are properly handled
    EXPECT_NE(json.find(R"(\"World\")"), std::string::npos);
    EXPECT_NE(json.find("Line 1"), std::string::npos);
    EXPECT_NE(json.find("世界"), std::string::npos);
    
    std::filesystem::remove("special_test.yaml");
}

TEST_F(IntegrationTest, CompleteWorkflow) {
    // Test the complete workflow as it would be used in main()
    const std::string input_file = "tests/test_data/nested.yaml";
    const std::string output_file = "test_output.json";
    
    // Read file
    auto content = FileReader::read_file(input_file);
    
    // Convert to JSON
    std::string json = YamlToJsonConverter::convert(content.data(), content.size(), input_file);
    
    // Pretty print
    std::string formatted = JsonFormatter::pretty_print(json);
    
    // Write to file
    std::ofstream output(output_file);
    output << formatted;
    output.close();
    
    // Verify output file exists and contains valid JSON
    ASSERT_TRUE(std::filesystem::exists(output_file));
    std::string output_content = readFile(output_file);
    EXPECT_FALSE(output_content.empty());
    EXPECT_EQ(output_content[0], '{'); // Should start with {
    EXPECT_NE(output_content.find('\n'), std::string::npos); // Should be pretty printed
    
    std::filesystem::remove(output_file);
}