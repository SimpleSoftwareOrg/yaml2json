#pragma once

#include <string>
#include <ryml.hpp>

namespace yaml2json {

// Core YAML to JSON converter
class YamlToJsonConverter {
public:
    // Convert YAML string to JSON string
    static std::string convert(const char* yaml_data, size_t yaml_size);
    
    // Convert YAML string to JSON string with filename for error reporting
    static std::string convert(const char* yaml_data, size_t yaml_size, const std::string& filename);
    
    // Parse YAML and return tree (for testing)
    static ryml::Tree parse_yaml(const char* yaml_data, size_t yaml_size, const std::string& filename = "");
    
    // Convert tree to JSON string
    static std::string tree_to_json(const ryml::Tree& tree);
};

} // namespace yaml2json