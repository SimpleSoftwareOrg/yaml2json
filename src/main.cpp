#include <ryml.hpp>
#include <ryml_std.hpp>
#include <iostream>
#include <string_view>
#include <fstream>

// Platform-specific includes for memory mapping (Unix only)
#ifdef _WIN32
    // Windows doesn't support mmap easily, so we'll use regular file I/O
#else
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

int main(int argc, char **argv) {
    // Disable synchronization with C I/O to speed up reading/writing
    std::ios::sync_with_stdio(false);

    // Load input: use mmap on Unix or regular file I/O, otherwise read stdin
    char *data_ptr = nullptr;
    size_t data_size = 0;
    std::string file_buffer;

#ifndef _WIN32
    int fd = -1;
    bool use_mmap = false;
#endif

    if(argc > 1 && std::string_view(argv[1]) != "-") {
        const char *path = argv[1];
        
#ifdef _WIN32
        // Windows: use regular file I/O
        std::ifstream file(path, std::ios::binary);
        if(file) {
            file.seekg(0, std::ios::end);
            data_size = static_cast<size_t>(file.tellg());
            file.seekg(0, std::ios::beg);
            
            file_buffer.resize(data_size);
            file.read(file_buffer.data(), data_size);
            data_ptr = file_buffer.data();
        }
#else
        // Unix: try mmap first, fallback to regular I/O
        fd = ::open(path, O_RDONLY);
        if(fd != -1) {
            struct stat st{};
            if(::fstat(fd, &st) == 0 && st.st_size > 0) {
                data_size = static_cast<size_t>(st.st_size);
                void *addr = ::mmap(nullptr, data_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
                if(addr != MAP_FAILED) {
                    data_ptr = static_cast<char*>(addr);
                    use_mmap = true;
                }
            }
        }
        
        if(!use_mmap) {
            // Fallback to regular file I/O
            std::ifstream file(path, std::ios::binary);
            if(file) {
                file.seekg(0, std::ios::end);
                data_size = static_cast<size_t>(file.tellg());
                file.seekg(0, std::ios::beg);
                
                file_buffer.resize(data_size);
                file.read(file_buffer.data(), data_size);
                data_ptr = file_buffer.data();
            }
        }
#endif
    }

    if(!data_ptr) {
        // Read from stdin if no file provided or file reading failed
        file_buffer.assign((std::istreambuf_iterator<char>(std::cin)),
                          std::istreambuf_iterator<char>());
        data_ptr = file_buffer.data();
        data_size = file_buffer.size();
    }

    // Zero-copy parse with pre-reserved capacity
    size_t est_nodes  = std::max<size_t>(1024, data_size / 90);
    size_t est_arena  = data_size * 11 / 10; // 1.1× YAML size

    ryml::Tree tree;
    tree.reserve(est_nodes);
    tree.reserve_arena(est_arena);

    ryml::substr yaml_sub(data_ptr, data_size);
    ryml::parse_in_place(yaml_sub, &tree);

    // Emit JSON (skip tag/anchor handling for compatibility)
    ryml::EmitOptions opts;
    opts.json_error_flags(static_cast<ryml::EmitOptions::EmitOptionFlags_e>(
        ryml::EmitOptions::JSON_ERR_ON_TAG | ryml::EmitOptions::JSON_ERR_ON_ANCHOR));

    ryml::emit_json(tree, opts, stdout);

    // Cleanup mmap if used
#ifndef _WIN32
    if(use_mmap) {
        ::munmap(data_ptr, data_size);
        ::close(fd);
    }
#endif
    return 0;
} 