#include <CLI/CLI.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <iterator>
#include <string>
#include <vector>

#include "FileReader.h"
#include "YamlToJsonConverter.h"
#include "JsonFormatter.h"
#include "ErrorHandler.h"
#include "BatchConverter.h"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

namespace {

// --batch: converts every named file, in order, and writes the JSON lines only when all of
// them are valid; otherwise names the first invalid file and writes nothing.
int run_batch(const std::string& input_file, const std::vector<std::string>& positional,
              const std::string& output_file, bool pretty_print) {
    if (pretty_print) {
        std::cerr << "Error: --pretty cannot be used with --batch" << std::endl;
        return 1;
    }
    std::vector<std::string> inputs;
    if (!input_file.empty()) inputs.push_back(input_file);
    inputs.insert(inputs.end(), positional.begin(), positional.end());
    if (inputs.empty()) {
        std::cerr << "Error: --batch needs at least one input file" << std::endl;
        return 1;
    }

    std::string lines;
    std::string content;
    for (const auto& name : inputs) {
        std::ifstream file(name, std::ios::binary);
        if (!file) {
            std::cerr << "Error: Cannot open input file '" << name
                      << "': " << std::strerror(errno) << std::endl;
            return 1;
        }
        content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        if (file.bad()) {
            std::cerr << "Error: Failed to read input file '" << name << "'" << std::endl;
            return 1;
        }
        try {
            yaml2json::BatchConverter::append_documents(lines, name, content.data(), content.size());
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }

    if (output_file.empty()) {
#ifdef _WIN32
        // Lines end in "\n" on every host.
        _setmode(_fileno(stdout), _O_BINARY);
#endif
        std::cout.write(lines.data(), static_cast<std::streamsize>(lines.size()));
        std::cout.flush();
        if (!std::cout.good()) {
            std::cerr << "Error: Failed to write to stdout" << std::endl;
            return 1;
        }
        return 0;
    }
    std::ofstream output(output_file, std::ios::binary);
    output.write(lines.data(), static_cast<std::streamsize>(lines.size()));
    if (!output.good()) {
        std::cerr << "Error: Failed to write to output file '" << output_file
                  << "': " << std::strerror(errno) << std::endl;
        return 1;
    }
    return 0;
}

} // namespace

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
    
    bool batch = false;
    app.add_flag("--batch", batch,
                 "Convert every input file named in one run and print one JSON line per YAML "
                 "document: {\"file\",\"doc\",\"duplicate_keys\",\"data\"}. Booleans and nulls "
                 "stay typed; every other scalar becomes a string holding its text");

    // Positional arguments for backwards compatibility
    app.add_option("files", positional_args,
                   "Input file [output file] (use stdin/stdout if omitted); with --batch, "
                   "the input files");
    
    // Version option
    app.set_version_flag("-v,--version", std::string("yaml2json built on ") + BUILD_DATE);
    
    // Parse command line
    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }
    
    if (batch) {
        return run_batch(input_file, positional_args, output_file, pretty_print);
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