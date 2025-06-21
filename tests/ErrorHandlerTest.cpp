#include <gtest/gtest.h>
#include "ErrorHandler.h"

using namespace yaml2json;

class ErrorHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset error handlers before each test
        setup_error_handlers();
    }
};

TEST_F(ErrorHandlerTest, FormatErrorWithLocation_AllFields) {
    std::string result = format_error_with_location(
        "Test error", 
        "test.yaml", 
        10, 
        5
    );
    EXPECT_EQ(result, "Test error in file 'test.yaml' at line 10, column 5");
}

TEST_F(ErrorHandlerTest, FormatErrorWithLocation_NoFilename) {
    std::string result = format_error_with_location(
        "Test error", 
        "", 
        10, 
        5
    );
    EXPECT_EQ(result, "Test error at line 10, column 5");
}

TEST_F(ErrorHandlerTest, FormatErrorWithLocation_NoLineColumn) {
    std::string result = format_error_with_location(
        "Test error", 
        "test.yaml", 
        0, 
        0
    );
    EXPECT_EQ(result, "Test error in file 'test.yaml'");
}

TEST_F(ErrorHandlerTest, ConversionError_Message) {
    ConversionError error("Test conversion error");
    EXPECT_STREQ(error.what(), "Test conversion error");
}

TEST_F(ErrorHandlerTest, RymlErrorHandler_ThrowsConversionError) {
    ryml::Location loc;
    loc.name = ryml::csubstr("test.yaml", 9);
    loc.line = 5;
    loc.col = 10;
    
    EXPECT_THROW({
        ryml_error_handler("Parse error", 11, loc, nullptr);
    }, ConversionError);
    
    try {
        ryml_error_handler("Parse error", 11, loc, nullptr);
    } catch (const ConversionError& e) {
        std::string expected = "YAML parsing error in file 'test.yaml' at line 5, column 10: Parse error";
        EXPECT_EQ(std::string(e.what()), expected);
    }
}