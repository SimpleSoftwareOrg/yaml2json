#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <vector>

class CliCompatibilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test YAML files
        createTestFile("test_simple.yaml", 
            "name: test\n"
            "version: 1.0\n"
            "enabled: true");
        
        createTestFile("test_nested.yaml",
            "database:\n"
            "  host: localhost\n"
            "  port: 5432\n"
            "api:\n"
            "  version: v1\n"
            "  timeout: 30");
    }
    
    void TearDown() override {
        // Clean up test files
        std::filesystem::remove("test_simple.yaml");
        std::filesystem::remove("test_nested.yaml");
        std::filesystem::remove("output_test.json");
        std::filesystem::remove("pretty_output.json");
    }
    
    void createTestFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        file << content;
        file.close();
    }
    
    std::string readFile(const std::string& filename) {
        std::ifstream file(filename);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    
    std::string runCommand(const std::string& command) {
        std::string result;
#ifdef _WIN32
        FILE* pipe = _popen(command.c_str(), "r");
#else
        FILE* pipe = popen(command.c_str(), "r");
#endif
        if (pipe) {
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                result += buffer;
            }
#ifdef _WIN32
            _pclose(pipe);
#else
            pclose(pipe);
#endif
        }
        return result;
    }
    
    bool fileExists(const std::string& filename) {
        return std::filesystem::exists(filename);
    }
    
    std::string getExecutablePath() {
#ifdef _WIN32
        return "..\\build\\yaml2json.exe";
#else
        return "../build/yaml2json";
#endif
    }
    
    std::string getCatCommand() {
#ifdef _WIN32
        return "type";
#else
        return "cat";
#endif
    }
    
    std::string getNullDevice() {
#ifdef _WIN32
        return "nul";
#else
        return "/dev/null";
#endif
    }
};

TEST_F(CliCompatibilityTest, PositionalArguments_InputAndOutput) {
    // Test: yaml2json input.yaml output.json
    std::string command = getExecutablePath() + " test_simple.yaml output_test.json";
    int result = system(command.c_str());
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(fileExists("output_test.json"));
    
    std::string output = readFile("output_test.json");
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("\"name\": \"test\""), std::string::npos);
    EXPECT_NE(output.find("\"version\": 1.0"), std::string::npos);
    EXPECT_NE(output.find("\"enabled\": true"), std::string::npos);
}

TEST_F(CliCompatibilityTest, PositionalArguments_InputOnly_OutputToStdout) {
    // Test: yaml2json input.yaml (output to stdout)
    std::string command = getExecutablePath() + " test_simple.yaml";
    std::string output = runCommand(command);
    
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("\"name\": \"test\""), std::string::npos);
    EXPECT_NE(output.find("\"version\": 1.0"), std::string::npos);
    EXPECT_NE(output.find("\"enabled\": true"), std::string::npos);
}

TEST_F(CliCompatibilityTest, StdinToStdout_Pipeline) {
    // Test: cat input.yaml | yaml2json (Unix) or type input.yaml | yaml2json (Windows)
    std::string command = getCatCommand() + " test_simple.yaml | " + getExecutablePath();
    std::string output = runCommand(command);
    
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("\"name\": \"test\""), std::string::npos);
    EXPECT_NE(output.find("\"version\": 1.0"), std::string::npos);
    EXPECT_NE(output.find("\"enabled\": true"), std::string::npos);
}

TEST_F(CliCompatibilityTest, StdinToFile_WithOutputFlag) {
    // Test: cat input.yaml | yaml2json --output output.json
    std::string command = getCatCommand() + " test_simple.yaml | " + getExecutablePath() + " --output output_test.json";
    int result = system(command.c_str());
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(fileExists("output_test.json"));
    
    std::string output = readFile("output_test.json");
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("\"name\": \"test\""), std::string::npos);
}

TEST_F(CliCompatibilityTest, FileToStdout_WithInputFlag) {
    // Test: yaml2json --input input.yaml (output to stdout)
    std::string command = getExecutablePath() + " --input test_simple.yaml";
    std::string output = runCommand(command);
    
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("\"name\": \"test\""), std::string::npos);
    EXPECT_NE(output.find("\"version\": 1.0"), std::string::npos);
}

TEST_F(CliCompatibilityTest, ExplicitFlags_InputAndOutput) {
    // Test: yaml2json --input input.yaml --output output.json (original method)
    std::string command = getExecutablePath() + " --input test_simple.yaml --output output_test.json";
    int result = system(command.c_str());
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(fileExists("output_test.json"));
    
    std::string output = readFile("output_test.json");
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("\"name\": \"test\""), std::string::npos);
}

TEST_F(CliCompatibilityTest, ShortFlags_InputAndOutput) {
    // Test: yaml2json -i input.yaml -o output.json
    std::string command = getExecutablePath() + " -i test_simple.yaml -o output_test.json";
    int result = system(command.c_str());
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(fileExists("output_test.json"));
    
    std::string output = readFile("output_test.json");
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("\"name\": \"test\""), std::string::npos);
}

TEST_F(CliCompatibilityTest, PrettyPrint_PositionalArgs) {
    // Test: yaml2json input.yaml output.json --pretty
    std::string command = getExecutablePath() + " test_nested.yaml pretty_output.json --pretty";
    int result = system(command.c_str());
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(fileExists("pretty_output.json"));
    
    std::string output = readFile("pretty_output.json");
    EXPECT_FALSE(output.empty());
    
    // Check for pretty printing (newlines and indentation)
    EXPECT_NE(output.find('\n'), std::string::npos);
    EXPECT_NE(output.find("  "), std::string::npos);
    EXPECT_NE(output.find("\"database\": {"), std::string::npos);
}

TEST_F(CliCompatibilityTest, PrettyPrint_StdoutMode) {
    // Test: yaml2json input.yaml --pretty (to stdout)
    std::string command = getExecutablePath() + " test_nested.yaml --pretty";
    std::string output = runCommand(command);
    
    EXPECT_FALSE(output.empty());
    
    // Check for pretty printing
    EXPECT_NE(output.find('\n'), std::string::npos);
    EXPECT_NE(output.find("  "), std::string::npos);
    EXPECT_NE(output.find("\"database\": {"), std::string::npos);
}

TEST_F(CliCompatibilityTest, PrettyPrint_Pipeline) {
    // Test: cat input.yaml | yaml2json --pretty
    std::string command = getCatCommand() + " test_nested.yaml | " + getExecutablePath() + " --pretty";
    std::string output = runCommand(command);
    
    EXPECT_FALSE(output.empty());
    
    // Check for pretty printing
    EXPECT_NE(output.find('\n'), std::string::npos);
    EXPECT_NE(output.find("  "), std::string::npos);
    EXPECT_NE(output.find("\"database\": {"), std::string::npos);
}

TEST_F(CliCompatibilityTest, MixedUsage_InputFlagPositionalOutput) {
    // Test: yaml2json --input input.yaml output.json
    std::string command = getExecutablePath() + " --input test_simple.yaml output_test.json";
    int result = system(command.c_str());
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(fileExists("output_test.json"));
    
    std::string output = readFile("output_test.json");
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("\"name\": \"test\""), std::string::npos);
}

TEST_F(CliCompatibilityTest, MixedUsage_PositionalInputOutputFlag) {
    // Test: yaml2json input.yaml --output output.json
    std::string command = getExecutablePath() + " test_simple.yaml --output output_test.json";
    int result = system(command.c_str());
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(fileExists("output_test.json"));
    
    std::string output = readFile("output_test.json");
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("\"name\": \"test\""), std::string::npos);
}

TEST_F(CliCompatibilityTest, ErrorCase_NonExistentFile) {
    // Test error handling for non-existent input file
    std::string command = getExecutablePath() + " non_existent.yaml 2>&1";
    std::string output = runCommand(command);
    
    EXPECT_NE(output.find("Error"), std::string::npos);
}

TEST_F(CliCompatibilityTest, ErrorCase_NoInputProvided) {
    // Test error when no input is provided and stdin is empty
    std::string command = getExecutablePath() + " < " + getNullDevice() + " 2>&1";
    std::string output = runCommand(command);
    
    // Should produce error message for empty input
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("Error"), std::string::npos);
    EXPECT_NE(output.find("No input provided"), std::string::npos);
}

TEST_F(CliCompatibilityTest, VersionFlag) {
    // Test --version flag works
    std::string command = getExecutablePath() + " --version";
    std::string output = runCommand(command);
    
    EXPECT_NE(output.find("yaml2json"), std::string::npos);
    EXPECT_NE(output.find("built on"), std::string::npos);
}

TEST_F(CliCompatibilityTest, HelpFlag) {
    // Test --help flag works
    std::string command = getExecutablePath() + " --help";
    std::string output = runCommand(command);
    
    EXPECT_NE(output.find("High-performance YAML to JSON converter"), std::string::npos);
    EXPECT_NE(output.find("--input"), std::string::npos);
    EXPECT_NE(output.find("--output"), std::string::npos);
    EXPECT_NE(output.find("--pretty"), std::string::npos);
}

TEST_F(CliCompatibilityTest, PriorityTest_ExplicitFlagsOverPositional) {
    // Test that explicit flags take priority over positional arguments
    // Create a second test file to distinguish
    createTestFile("test_priority.yaml", "priority: explicit_flags\nvalue: 999\n");
    
    // This should use test_priority.yaml (from --input) not test_simple.yaml (positional)
    std::string command = getExecutablePath() + " test_simple.yaml --input test_priority.yaml --output output_test.json";
    int result = system(command.c_str());
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(fileExists("output_test.json"));
    
    std::string output = readFile("output_test.json");
    EXPECT_NE(output.find("\"priority\": \"explicit_flags\""), std::string::npos);
    EXPECT_NE(output.find("\"value\": 999"), std::string::npos);
    
    // Clean up
    std::filesystem::remove("test_priority.yaml");
}

TEST_F(CliCompatibilityTest, ComplexNestedStructure_AllModes) {
    // Create a more complex YAML for thorough testing
    createTestFile("complex_test.yaml",
        "application:\n"
        "  name: yaml2json\n"
        "  version: 2.0\n"
        "servers:\n"
        "  - name: web1\n"
        "    port: 8080\n"
        "    enabled: true\n"
        "  - name: web2\n"
        "    port: 8081\n"
        "    enabled: false\n"
        "config:\n"
        "  timeout: 30\n"
        "  retries: 3\n"
        "  features:\n"
        "    - fast_parsing\n"
        "    - pretty_print\n");
    
    // Test all modes with complex structure
    std::vector<std::string> commands = {
        getExecutablePath() + " complex_test.yaml",  // File to stdout
        getCatCommand() + " complex_test.yaml | " + getExecutablePath(),  // Stdin to stdout
        getExecutablePath() + " --input complex_test.yaml",  // Explicit input flag
    };
    
    for (const auto& command : commands) {
        std::string output = runCommand(command);
        
        EXPECT_FALSE(output.empty());
        EXPECT_NE(output.find("\"application\""), std::string::npos);
        EXPECT_NE(output.find("\"servers\""), std::string::npos);
        EXPECT_NE(output.find("\"web1\""), std::string::npos);
        EXPECT_NE(output.find("\"features\""), std::string::npos);
        EXPECT_NE(output.find("\"fast_parsing\""), std::string::npos);
    }
    
    // Clean up
    std::filesystem::remove("complex_test.yaml");
} 