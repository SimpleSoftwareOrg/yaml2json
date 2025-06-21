#include <ryml.hpp>
#include <ryml_std.hpp>
#include <CLI/CLI.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <cerrno>
#include <cstring>

// Platform-specific includes for memory mapping (Unix only)
#ifdef _WIN32
    // Windows doesn't support mmap easily, so we'll use regular file I/O
#else
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

// Error handling with verbose messages
[[noreturn]] void error_exit(const std::string& message) {
    std::cerr << "Error: " << message << std::endl;
    std::exit(1);
}

// Custom error handler for rapidyaml
void ryml_error_handler(const char* msg, size_t msg_len, ryml::Location loc, void* /*user_data*/) {
    std::string error_msg = "YAML parsing error";
    if (loc.name.not_empty()) {
        error_msg += " in file '";
        error_msg.append(loc.name.str, loc.name.len);
        error_msg += "'";
    }
    error_msg += " at line ";
    error_msg += std::to_string(loc.line);
    error_msg += ", column ";
    error_msg += std::to_string(loc.col);
    error_msg += ": ";
    error_msg.append(msg, msg_len);
    
    error_exit(error_msg);
}

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
    
    // Load input file
    char *data_ptr = nullptr;
    size_t data_size = 0;
    std::string file_buffer;

#ifndef _WIN32
    int fd = -1;
    bool use_mmap = false;
#endif

    // Check if input file exists and is readable
    if (!std::filesystem::exists(input_file)) {
        error_exit("Input file '" + input_file + "' does not exist");
    }
    
    if (!std::filesystem::is_regular_file(input_file)) {
        error_exit("Input path '" + input_file + "' is not a regular file");
    }
    
    // Try to read the input file
#ifdef _WIN32
    // Windows: use regular file I/O
    std::ifstream file(input_file, std::ios::binary);
    if (!file) {
        error_exit("Failed to open input file '" + input_file + "': " + std::strerror(errno));
    }
    
    file.seekg(0, std::ios::end);
    data_size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);
    
    if (data_size == 0) {
        error_exit("Input file '" + input_file + "' is empty");
    }
    
    file_buffer.resize(data_size);
    if (!file.read(file_buffer.data(), data_size)) {
        error_exit("Failed to read input file '" + input_file + "': " + std::strerror(errno));
    }
    data_ptr = file_buffer.data();
#else
    // Unix: try mmap first, fallback to regular I/O
    fd = ::open(input_file.c_str(), O_RDONLY);
    if (fd == -1) {
        error_exit("Failed to open input file '" + input_file + "': " + std::strerror(errno));
    }
    
    struct stat st{};
    if (::fstat(fd, &st) != 0) {
        ::close(fd);
        error_exit("Failed to stat input file '" + input_file + "': " + std::strerror(errno));
    }
    
    if (st.st_size == 0) {
        ::close(fd);
        error_exit("Input file '" + input_file + "' is empty");
    }
    
    data_size = static_cast<size_t>(st.st_size);
    void *addr = ::mmap(nullptr, data_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (addr != MAP_FAILED) {
        data_ptr = static_cast<char*>(addr);
        use_mmap = true;
    } else {
        // Fallback to regular file I/O
        std::ifstream file(input_file, std::ios::binary);
        if (!file) {
            ::close(fd);
            error_exit("Failed to open input file '" + input_file + "': " + std::strerror(errno));
        }
        
        file_buffer.resize(data_size);
        if (!file.read(file_buffer.data(), data_size)) {
            ::close(fd);
            error_exit("Failed to read input file '" + input_file + "': " + std::strerror(errno));
        }
        data_ptr = file_buffer.data();
    }
#endif

    // Parse YAML
    try {
        // Set custom error handler
        ryml::set_callbacks(ryml::Callbacks(nullptr, nullptr, nullptr, ryml_error_handler));
        
        // Zero-copy parse with pre-reserved capacity
        size_t est_nodes = std::max<size_t>(1024, data_size / 90);
        size_t est_arena = data_size * 11 / 10; // 1.1× YAML size

        ryml::Tree tree;
        tree.reserve(est_nodes);
        tree.reserve_arena(est_arena);

        ryml::substr yaml_sub(data_ptr, data_size);
        ryml::parse_in_place(yaml_sub, &tree);

        // Open output file
        std::ofstream output(output_file, std::ios::binary);
        if (!output) {
            error_exit("Failed to create output file '" + output_file + "': " + std::strerror(errno));
        }

        // Emit JSON
        if (pretty_print) {
            // Pretty-print to string first, then write
            std::string json_output = ryml::emitrs_json<std::string>(tree);
            
            // Simple pretty-printing: add newlines and indentation
            std::string pretty_output;
            pretty_output.reserve(json_output.size() * 2);
            
            int indent_level = 0;
            bool in_string = false;
            bool escape_next = false;
            
            for (size_t i = 0; i < json_output.size(); ++i) {
                char c = json_output[i];
                
                if (escape_next) {
                    pretty_output += c;
                    escape_next = false;
                    continue;
                }
                
                if (c == '\\') {
                    pretty_output += c;
                    escape_next = true;
                    continue;
                }
                
                if (c == '"' && !in_string) {
                    in_string = true;
                    pretty_output += c;
                    continue;
                } else if (c == '"' && in_string) {
                    in_string = false;
                    pretty_output += c;
                    continue;
                }
                
                if (in_string) {
                    pretty_output += c;
                    continue;
                }
                
                switch (c) {
                    case '{':
                    case '[':
                        pretty_output += c;
                        pretty_output += '\n';
                        indent_level++;
                        for (int j = 0; j < indent_level * 2; ++j) {
                            pretty_output += ' ';
                        }
                        break;
                    case '}':
                    case ']':
                        if (!pretty_output.empty() && pretty_output.back() == ' ') {
                            // Remove trailing spaces
                            while (!pretty_output.empty() && pretty_output.back() == ' ') {
                                pretty_output.pop_back();
                            }
                        }
                        if (!pretty_output.empty() && pretty_output.back() != '\n') {
                            pretty_output += '\n';
                        }
                        indent_level--;
                        for (int j = 0; j < indent_level * 2; ++j) {
                            pretty_output += ' ';
                        }
                        pretty_output += c;
                        break;
                    case ',':
                        pretty_output += c;
                        pretty_output += '\n';
                        for (int j = 0; j < indent_level * 2; ++j) {
                            pretty_output += ' ';
                        }
                        break;
                    case ':':
                        pretty_output += c;
                        pretty_output += ' ';
                        break;
                    default:
                        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                            pretty_output += c;
                        }
                        break;
                }
            }
            
            // Add final newline if not present
            if (!pretty_output.empty() && pretty_output.back() != '\n') {
                pretty_output += '\n';
            }
            
            output.write(pretty_output.data(), pretty_output.size());
        } else {
            // Compact output
            std::string json_output = ryml::emitrs_json<std::string>(tree);
            output.write(json_output.data(), json_output.size());
        }
        
        if (!output.good()) {
            error_exit("Failed to write to output file '" + output_file + "': " + std::strerror(errno));
        }
        
        output.close();
        
    } catch (const std::exception& e) {
        error_exit("YAML parsing error in file '" + input_file + "': " + e.what());
    }

    // Cleanup mmap if used
#ifndef _WIN32
    if (use_mmap) {
        ::munmap(data_ptr, data_size);
    }
    if (fd != -1) {
        ::close(fd);
    }
#endif
    
    return 0;
} 