#include <CLI/CLI.hpp>
#include <iostream>
#include <fstream>
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
    
    // Input and output file options (required)
    std::string input_file;
    std::string output_file;
    bool pretty_print = false;
    
    app.add_option("-i,--input", input_file, "Input YAML file")
        ->required()
        ->check(CLI::ExistingFile);
    
    app.add_option("-o,--output", output_file, "Output JSON file")
        ->required();
    
    app.add_flag("-p,--pretty", pretty_print, "Pretty-print JSON output with indentation");
    
    // Version option
    app.set_version_flag("-v,--version", std::string("yaml2json built on ") + BUILD_DATE);
    
    // Parse command line
    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }
    
    try {
        // Read input file
        auto file_content = yaml2json::FileReader::read_file(input_file);
        
        // Convert YAML to JSON
        std::string json_output = yaml2json::YamlToJsonConverter::convert(
            file_content.mutable_data(), 
            file_content.size(), 
            input_file
        );
        
        // Format JSON if requested
        if (pretty_print) {
            json_output = yaml2json::JsonFormatter::pretty_print(json_output);
        }
        
        // Write output file
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
        
    } catch (const yaml2json::ConversionError& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: Unexpected error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 