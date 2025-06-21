#pragma once

#include <string>
#include <memory>
#include <cstddef>

namespace yaml2json {

// RAII wrapper for file content
class FileContent {
public:
    FileContent() = default;
    FileContent(const FileContent&) = delete;
    FileContent& operator=(const FileContent&) = delete;
    FileContent(FileContent&&) = default;
    FileContent& operator=(FileContent&&) = default;
    ~FileContent();

    // Get pointer to data
    const char* data() const { return data_ptr_; }
    char* mutable_data() { return data_ptr_; }
    
    // Get size of data
    size_t size() const { return size_; }
    
    // Check if content is valid
    bool is_valid() const { return data_ptr_ != nullptr && size_ > 0; }

private:
    friend class FileReader;
    
    char* data_ptr_ = nullptr;
    size_t size_ = 0;
    bool is_mmap_ = false;
    int fd_ = -1;
    std::unique_ptr<char[]> owned_data_;
};

// File reader with memory mapping support
class FileReader {
public:
    // Read file content (uses mmap on Unix, regular I/O on Windows)
    static FileContent read_file(const std::string& filepath);
    
    // Check if file exists and is readable
    static void validate_file(const std::string& filepath);
};

} // namespace yaml2json