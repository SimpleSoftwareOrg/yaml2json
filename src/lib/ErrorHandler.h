#pragma once

#include <string>
#include <stdexcept>
#include <ryml.hpp>

namespace yaml2json {

// Custom exception for YAML to JSON conversion errors
class ConversionError : public std::runtime_error {
public:
    explicit ConversionError(const std::string& message) 
        : std::runtime_error(message) {}
};

// Error handler for rapidyaml callbacks
void ryml_error_handler(const char* msg, size_t msg_len, ryml::Location loc, void* user_data);

// Set up error handlers for rapidyaml
void setup_error_handlers();

// Format error message with location information
std::string format_error_with_location(const std::string& base_message, 
                                       const std::string& filename,
                                       size_t line, 
                                       size_t column);

} // namespace yaml2json