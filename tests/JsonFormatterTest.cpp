#include <gtest/gtest.h>
#include "JsonFormatter.h"

using namespace yaml2json;

class JsonFormatterTest : public ::testing::Test {
};

TEST_F(JsonFormatterTest, PrettyPrint_SimpleObject) {
    std::string input = R"({"name":"test","value":42})";
    std::string result = JsonFormatter::pretty_print(input);
    
    std::string expected = "{\n  \"name\": \"test\",\n  \"value\": 42\n}\n";
    EXPECT_EQ(result, expected);
}

TEST_F(JsonFormatterTest, PrettyPrint_NestedObject) {
    std::string input = R"({"parent":{"child":"value","number":123}})";
    std::string result = JsonFormatter::pretty_print(input);
    
    std::string expected = "{\n  \"parent\": {\n    \"child\": \"value\",\n    \"number\": 123\n  }\n}\n";
    EXPECT_EQ(result, expected);
}

TEST_F(JsonFormatterTest, PrettyPrint_Array) {
    std::string input = R"({"items":["first","second","third"]})";
    std::string result = JsonFormatter::pretty_print(input);
    
    std::string expected = "{\n  \"items\": [\n    \"first\",\n    \"second\",\n    \"third\"\n  ]\n}\n";
    EXPECT_EQ(result, expected);
}

TEST_F(JsonFormatterTest, PrettyPrint_EmptyObject) {
    std::string input = "{}";
    std::string result = JsonFormatter::pretty_print(input);
    
    EXPECT_EQ(result, "{\n}\n");
}

TEST_F(JsonFormatterTest, PrettyPrint_EmptyArray) {
    std::string input = "[]";
    std::string result = JsonFormatter::pretty_print(input);
    
    EXPECT_EQ(result, "[\n]\n");
}

TEST_F(JsonFormatterTest, PrettyPrint_StringWithSpecialChars) {
    std::string input = R"({"text":"Hello \"World\"\nNew line"})";
    std::string result = JsonFormatter::pretty_print(input);
    
    // The formatter should preserve escaped characters within strings
    EXPECT_NE(result.find(R"(\"World\")"), std::string::npos);
    EXPECT_NE(result.find(R"(\n)"), std::string::npos);
}

TEST_F(JsonFormatterTest, Compact_AlreadyCompact) {
    std::string input = R"({"name":"test","value":42})";
    std::string result = JsonFormatter::compact(input);
    
    EXPECT_EQ(result, input);
}

TEST_F(JsonFormatterTest, Format_WithCustomOptions) {
    JsonFormatOptions options;
    options.pretty_print = true;
    options.indent_size = 4;
    options.indent_char = ' ';
    options.add_final_newline = false;
    
    std::string input = R"({"name":"test"})";
    std::string result = JsonFormatter::format(input, options);
    
    std::string expected = "{\n    \"name\": \"test\"\n}";
    EXPECT_EQ(result, expected);
}

TEST_F(JsonFormatterTest, Format_CompactMode) {
    JsonFormatOptions options;
    options.pretty_print = false;
    
    std::string input = R"({"name":"test","value":42})";
    std::string result = JsonFormatter::format(input, options);
    
    EXPECT_EQ(result, input);
}

TEST_F(JsonFormatterTest, PrettyPrint_ComplexNesting) {
    std::string input = R"({"a":{"b":{"c":[1,2,{"d":"e"}]}}})";
    std::string result = JsonFormatter::pretty_print(input);
    
    // Just verify it doesn't crash and produces formatted output
    EXPECT_GT(result.length(), input.length());
    EXPECT_NE(result.find('\n'), std::string::npos);
}