#include <gtest/gtest.h>
#include <cstring>
#include "YamlToJsonConverter.h"
#include "ErrorHandler.h"

using namespace yaml2json;

class YamlToJsonConverterTest : public ::testing::Test {
protected:
    void SetUp() override {
        setup_error_handlers();
    }
};

TEST_F(YamlToJsonConverterTest, Convert_SimpleYaml) {
    const char* yaml = "name: test\nvalue: 42";
    std::string json = YamlToJsonConverter::convert(yaml, strlen(yaml));
    
    EXPECT_EQ(json, R"({"name": "test","value": 42})");
}

TEST_F(YamlToJsonConverterTest, Convert_NestedYaml) {
    const char* yaml = "parent:\n  child: value\n  number: 123";
    std::string json = YamlToJsonConverter::convert(yaml, strlen(yaml));
    
    EXPECT_EQ(json, R"({"parent": {"child": "value","number": 123}})");
}

TEST_F(YamlToJsonConverterTest, Convert_ArrayYaml) {
    const char* yaml = "items:\n  - first\n  - second\n  - third";
    std::string json = YamlToJsonConverter::convert(yaml, strlen(yaml));
    
    EXPECT_EQ(json, R"({"items": ["first","second","third"]})");
}

TEST_F(YamlToJsonConverterTest, Convert_MixedTypes) {
    const char* yaml = "string: hello\ninteger: 42\nfloat: 3.14\nbool: true\nnull_val: null";
    std::string json = YamlToJsonConverter::convert(yaml, strlen(yaml));
    
    // Note: The order might vary, so we check for presence of each field
    EXPECT_NE(json.find(R"("string": "hello")"), std::string::npos);
    EXPECT_NE(json.find(R"("integer": 42)"), std::string::npos);
    EXPECT_NE(json.find(R"("float": 3.14)"), std::string::npos);
    EXPECT_NE(json.find(R"("bool": true)"), std::string::npos);
    EXPECT_NE(json.find(R"("null_val": null)"), std::string::npos);
}

TEST_F(YamlToJsonConverterTest, Convert_EmptyYaml) {
    const char* yaml = "";
    std::string json = YamlToJsonConverter::convert(yaml, 0);
    
    // Empty YAML produces empty string or empty object
    EXPECT_TRUE(json.empty() || json == "{}" || json == "null");
}

TEST_F(YamlToJsonConverterTest, Convert_InvalidYaml) {
    // Use a simpler invalid YAML that's less likely to cause a hard crash
    const char* yaml = "key: [unclosed bracket";
    
    EXPECT_THROW({
        YamlToJsonConverter::convert(yaml, strlen(yaml));
    }, ConversionError);
}

TEST_F(YamlToJsonConverterTest, Convert_WithFilename) {
    const char* yaml = "test: value";
    std::string json = YamlToJsonConverter::convert(yaml, strlen(yaml), "test.yaml");
    
    EXPECT_EQ(json, R"({"test": "value"})");
}

TEST_F(YamlToJsonConverterTest, ParseYaml_ValidInput) {
    const char* yaml = "key: value";
    
    EXPECT_NO_THROW({
        ryml::Tree tree = YamlToJsonConverter::parse_yaml(yaml, strlen(yaml));
        EXPECT_TRUE(tree.rootref().has_children());
    });
}

TEST_F(YamlToJsonConverterTest, TreeToJson_SimpleTree) {
    const char* yaml = "test: 123";
    ryml::Tree tree = YamlToJsonConverter::parse_yaml(yaml, strlen(yaml));
    std::string json = YamlToJsonConverter::tree_to_json(tree);
    
    EXPECT_EQ(json, R"({"test": 123})");
}

TEST_F(YamlToJsonConverterTest, Convert_SpecialCharacters) {
    const char* yaml = "text: \"Hello World\"";
    std::string json = YamlToJsonConverter::convert(yaml, strlen(yaml));
    
    // Check that the string is properly quoted in JSON
    EXPECT_NE(json.find(R"("text": "Hello World")"), std::string::npos);
}