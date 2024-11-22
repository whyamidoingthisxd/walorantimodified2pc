#include "utilities/config.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

// Constructor
ConfigHandler::ConfigHandler(const std::string& file_path) : file_path(file_path) {}

// Load configuration from file
bool ConfigHandler::load() {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << file_path << std::endl;
        return false;
    }

    std::string line, section;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;
        if (line[0] == '[') {
            section = line.substr(1, line.find(']') - 1);
        }
        else {
            size_t eq_pos = line.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = line.substr(0, eq_pos);
                std::string value = line.substr(eq_pos + 1);

                config_data[section][key] = value;
            }
        }
    }
    file.close();
    return true;
}

// Save configuration to file
bool ConfigHandler::save() {
    std::ofstream file(file_path, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Failed to save config file: " << file_path << std::endl;
        return false;
    }

    for (const auto& section : config_data) {
        file << "[" << section.first << "]\n";
        for (const auto& key_value : section.second) {
            file << key_value.first << "=" << key_value.second << "\n";
        }
        file << "\n";
    }
    file.close();
    return true;
}

// Get an integer value from the configuration
int ConfigHandler::get_int(const std::string& section, const std::string& key, int default_value) {
    if (config_data.count(section) && config_data[section].count(key)) {
        try {
            return std::stoi(config_data[section][key]);
        }
        catch (...) {
            return default_value;
        }
    }
    return default_value;
}

// Get a string value from the configuration
std::string ConfigHandler::get(const std::string& section, const std::string& key, const std::string& default_value) {
    if (config_data.count(section) && config_data[section].count(key)) {
        return config_data[section][key];
    }
    return default_value;
}

// Set a string value in the configuration
void ConfigHandler::set(const std::string& section, const std::string& key, const std::string& value) {
    config_data[section][key] = value;
}

// Set an integer value in the configuration
void ConfigHandler::set_int(const std::string& section, const std::string& key, int value) {
    set(section, key, std::to_string(value));
}
