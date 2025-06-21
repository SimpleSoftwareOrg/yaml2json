#include "FileReader.h"
#include "ErrorHandler.h"
#include <filesystem>
#include <fstream>
#include <cerrno>
#include <cstring>

#ifdef _WIN32
    // Windows doesn't support mmap easily, so we'll use regular file I/O
#else
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

namespace yaml2json {

FileContent::~FileContent() {
#ifndef _WIN32
    if (is_mmap_ && data_ptr_) {
        ::munmap(data_ptr_, size_);
    }
    if (fd_ != -1) {
        ::close(fd_);
    }
#endif
}

void FileReader::validate_file(const std::string& filepath) {
    if (!std::filesystem::exists(filepath)) {
        throw ConversionError("Input file '" + filepath + "' does not exist");
    }
    
    if (!std::filesystem::is_regular_file(filepath)) {
        throw ConversionError("Input path '" + filepath + "' is not a regular file");
    }
}

FileContent FileReader::read_file(const std::string& filepath) {
    validate_file(filepath);
    
    FileContent content;
    
#ifdef _WIN32
    // Windows: use regular file I/O
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        throw ConversionError("Failed to open input file '" + filepath + "': " + std::strerror(errno));
    }
    
    file.seekg(0, std::ios::end);
    content.size_ = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);
    
    if (content.size_ == 0) {
        throw ConversionError("Input file '" + filepath + "' is empty");
    }
    
    content.owned_data_ = std::make_unique<char[]>(content.size_);
    if (!file.read(content.owned_data_.get(), content.size_)) {
        throw ConversionError("Failed to read input file '" + filepath + "': " + std::strerror(errno));
    }
    content.data_ptr_ = content.owned_data_.get();
#else
    // Unix: try mmap first, fallback to regular I/O
    content.fd_ = ::open(filepath.c_str(), O_RDONLY);
    if (content.fd_ == -1) {
        throw ConversionError("Failed to open input file '" + filepath + "': " + std::strerror(errno));
    }
    
    struct stat st{};
    if (::fstat(content.fd_, &st) != 0) {
        ::close(content.fd_);
        content.fd_ = -1;
        throw ConversionError("Failed to stat input file '" + filepath + "': " + std::strerror(errno));
    }
    
    if (st.st_size == 0) {
        ::close(content.fd_);
        content.fd_ = -1;
        throw ConversionError("Input file '" + filepath + "' is empty");
    }
    
    content.size_ = static_cast<size_t>(st.st_size);
    void *addr = ::mmap(nullptr, content.size_, PROT_READ | PROT_WRITE, MAP_PRIVATE, content.fd_, 0);
    if (addr != MAP_FAILED) {
        content.data_ptr_ = static_cast<char*>(addr);
        content.is_mmap_ = true;
    } else {
        // Fallback to regular file I/O
        std::ifstream file(filepath, std::ios::binary);
        if (!file) {
            ::close(content.fd_);
            content.fd_ = -1;
            throw ConversionError("Failed to open input file '" + filepath + "': " + std::strerror(errno));
        }
        
        content.owned_data_ = std::make_unique<char[]>(content.size_);
        if (!file.read(content.owned_data_.get(), content.size_)) {
            ::close(content.fd_);
            content.fd_ = -1;
            throw ConversionError("Failed to read input file '" + filepath + "': " + std::strerror(errno));
        }
        content.data_ptr_ = content.owned_data_.get();
    }
#endif
    
    return content;
}

} // namespace yaml2json