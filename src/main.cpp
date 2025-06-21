#include <CLI/CLI.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include "FileReader.h"
#include "YamlToJsonConverter.h"
#include "JsonFormatter.h"
#include "ErrorHandler.h"

int main(int argc, char **argv) {
    // Disable synchronization with C I/O to speed up reading/writing
    std::ios::sync_with_stdio(false);

    // CLI setup
    CLI::App app{"High-performance YAML to JSON converter"};
    
    // Input and output file options (optional for backwards compatibility)
    std::string input_file;
    std::string output_file;
    bool pretty_print = false;
    std::vector<std::string> positional_args;
    
    // Optional flags for explicit file specification
    app.add_option("-i,--input", input_file, "Input YAML file")
        ->check(CLI::ExistingFile);
    
    app.add_option("-o,--output", output_file, "Output JSON file");
    
    app.add_flag("-p,--pretty", pretty_print, "Pretty-print JSON output with indentation");
    
    // Positional arguments for backwards compatibility
    app.add_option("files", positional_args, "Input file [output file] (use stdin/stdout if omitted)");
    
    // Version option
    app.set_version_flag("-v,--version", std::string("yaml2json built on ") + BUILD_DATE);
    
    // Parse command line
    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }
    
    // Determine input and output sources with backwards compatibility
    bool use_stdin = false;
    bool use_stdout = false;
    
    // Handle positional arguments (can supplement explicit flags)
    if (!positional_args.empty()) {
        // If no explicit input, use first positional arg as input
        if (input_file.empty()) {
            input_file = positional_args[0];
        }
        
        // If no explicit output, use second positional arg as output (if available)
        // This works whether input came from flag or positional
        if (output_file.empty()) {
            if (positional_args.size() > 1) {
                output_file = positional_args[1];
            } else if (positional_args.size() == 1 && !input_file.empty() && input_file != positional_args[0]) {
                // Case: --input file.yaml output.json (single positional is output)
                output_file = positional_args[0];
            }
        }
    }
    
    // If no input specified, use stdin
    if (input_file.empty()) {
        use_stdin = true;
    }
    
    // If no output specified, use stdout
    if (output_file.empty()) {
        use_stdout = true;
    }
    
    // Validate input file exists (if not using stdin)
    if (!use_stdin) {
        std::ifstream test_input(input_file);
        if (!test_input.good()) {
            std::cerr << "Error: Cannot open input file '" << input_file 
                     << "': " << std::strerror(errno) << std::endl;
            return 1;
        }
    }
    
    try {
        // Read input (file or stdin)
        std::string yaml_content;
        std::string source_name;
        
        if (use_stdin) {
            // Read from stdin
            std::ostringstream buffer;
            buffer << std::cin.rdbuf();
            yaml_content = buffer.str();
            source_name = "<stdin>";
            
            if (yaml_content.empty()) {
                std::cerr << "Error: No input provided via stdin" << std::endl;
                return 1;
            }
        } else {
            // Read from file using FileReader
            auto file_content = yaml2json::FileReader::read_file(input_file);
            yaml_content.assign(file_content.data(), file_content.size());
            source_name = input_file;
        }
        
        // Convert YAML to JSON
        std::string json_output = yaml2json::YamlToJsonConverter::convert(
            const_cast<char*>(yaml_content.data()), 
            yaml_content.size(), 
            source_name
        );
        
        // Format JSON if requested
        if (pretty_print) {
            json_output = yaml2json::JsonFormatter::pretty_print(json_output);
        }
        
        // Write output (file or stdout)
        if (use_stdout) {
            // Write to stdout
            std::cout.write(json_output.data(), json_output.size());
            std::cout.flush();
            
            if (!std::cout.good()) {
                std::cerr << "Error: Failed to write to stdout" << std::endl;
                return 1;
            }
        } else {
            // Write to file
            std::ofstream output(output_file, std::ios::binary);
            if (!output) {
                std::cerr << "Error: Failed to create output file '" << output_file 
                         << "': " << std::strerror(errno) << std::endl;
                return 1;
            }
            
            output.write(json_output.data(), json_output.size());
            
            if (!output.good()) {
                std::cerr << "Error: Failed to write to output file '" << output_file 
                         << "': " << std::strerror(errno) << std::endl;
                return 1;
            }
        }
        
    } catch (const yaml2json::ConversionError& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: Unexpected error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 