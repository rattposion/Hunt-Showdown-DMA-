#include "Config.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <windows.h>

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

void Config::SaveConfig(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to save config to " << filename << std::endl;
        return;
    }

    file << "[Overlay]\n";
    file << "show_menu=" << overlay.show_menu << "\n";
    file << "vsync=" << overlay.vsync << "\n";
    file << "show_fps=" << overlay.show_fps << "\n";
    file << "show_cursor_dot=" << overlay.show_cursor_dot << "\n";
    file << "menu_alpha=" << overlay.menu_alpha << "\n";
    file << "\n";

    file << "[Features]\n";
    file << "aimbot=" << features.aimbot << "\n";
    file << "esp=" << features.esp << "\n";
    file << "aimbot_smoothing=" << features.aimbot_smoothing << "\n";
    file << "aimbot_fov=" << features.aimbot_fov << "\n";
    file << "aimbot_deadzone=" << features.aimbot_deadzone << "\n";
    file << "aimbot_head_targeting=" << features.aimbot_head_targeting << "\n";
    file << "aimbot_key=" << features.aimbot_key << "\n";
    file << "\n";

    file << "[Visuals]\n";
    file << "show_health=" << visuals.show_health << "\n";
    file << "show_enemies=" << visuals.show_enemies << "\n";
    file << "draw_fov_circle=" << visuals.draw_fov_circle << "\n";
    file << "fov_circle_radius=" << visuals.fov_circle_radius << "\n";
    file << "crosshair=" << visuals.crosshair << "\n";
    file << "crosshair_size=" << visuals.crosshair_size << "\n";
    file << "\n";

    file << "[Colors]\n";
    file << "enemy_color=" << colors.enemy_color[0] << "," << colors.enemy_color[1] << "," << colors.enemy_color[2] << "\n";
    file << "friendly_color=" << colors.friendly_color[0] << "," << colors.friendly_color[1] << "," << colors.friendly_color[2] << "\n";
    file << "item_color=" << colors.item_color[0] << "," << colors.item_color[1] << "," << colors.item_color[2] << "\n";
    file << "crosshair_color=" << colors.crosshair_color[0] << "," << colors.crosshair_color[1] << "," << colors.crosshair_color[2] << "\n";
    file << "\n";

    file << "[Hotkeys]\n";
    file << "toggle_menu=" << hotkeys.toggle_menu << "\n";
    file << "toggle_aimbot=" << hotkeys.toggle_aimbot << "\n";
    file << "toggle_esp=" << hotkeys.toggle_esp << "\n";


    file.close();
    std::cout << "Config saved to " << filename << std::endl;
}

void Config::LoadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Config file not found, using defaults." << std::endl;
        return;
    }

    std::string line, section;
    while (std::getline(file, line)) {
        // Remove whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;

        // Check for section headers
        if (line[0] == '[' && line.back() == ']') {
            section = line.substr(1, line.length() - 2);
            continue;
        }

        // Parse key=value pairs
        size_t pos = line.find('=');
        if (pos == std::string::npos)
            continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        // Parse values based on section
        if (section == "Overlay") {
            if (key == "show_menu") overlay.show_menu = (value == "1");
            else if (key == "vsync") overlay.vsync = (value == "1");
            else if (key == "show_fps") overlay.show_fps = (value == "1");
            else if (key == "show_cursor_dot") overlay.show_cursor_dot = (value == "1");
            else if (key == "menu_alpha") overlay.menu_alpha = std::stof(value);
        }
        else if (section == "Features") {
            if (key == "aimbot") features.aimbot = (value == "1");
            else if (key == "esp") features.esp = (value == "1");
            else if (key == "aimbot_smoothing") features.aimbot_smoothing = std::stof(value);
            else if (key == "aimbot_fov") features.aimbot_fov = std::stof(value);
            else if (key == "aimbot_deadzone") features.aimbot_deadzone = std::stoi(value);
            else if (key == "aimbot_head_targeting") features.aimbot_head_targeting = (value == "1");
            else if (key == "aimbot_key") features.aimbot_key = std::stoi(value);
        }
        else if (section == "Visuals") {
            if (key == "show_health") visuals.show_health = (value == "1");
            else if (key == "show_enemies") visuals.show_enemies = (value == "1");
            else if (key == "show_bosses") visuals.show_bosses = (value == "1");
            else if (key == "draw_fov_circle") visuals.draw_fov_circle = (value == "1");
            else if (key == "fov_circle_radius") visuals.fov_circle_radius = std::stof(value);
            else if (key == "crosshair") visuals.crosshair = (value == "1");
            else if (key == "crosshair_size") visuals.crosshair_size = std::stof(value);
        }
        else if (section == "Colors") {
            if (key == "enemy_color") {
                std::stringstream ss(value);
                std::string item;
                int i = 0;
                while (std::getline(ss, item, ',') && i < 3) {
                    colors.enemy_color[i++] = std::stof(item);
                }
            }
            else if (key == "friendly_color") {
                std::stringstream ss(value);
                std::string item;
                int i = 0;
                while (std::getline(ss, item, ',') && i < 3) {
                    colors.friendly_color[i++] = std::stof(item);
                }
            }
            else if (key == "item_color") {
                std::stringstream ss(value);
                std::string item;
                int i = 0;
                while (std::getline(ss, item, ',') && i < 3) {
                    colors.item_color[i++] = std::stof(item);
                }
            }
            else if (key == "crosshair_color") {
                std::stringstream ss(value);
                std::string item;
                int i = 0;
                while (std::getline(ss, item, ',') && i < 3) {
                    colors.crosshair_color[i++] = std::stof(item);
                }
            }
        }
        else if (section == "Hotkeys") {
            if (key == "toggle_menu") hotkeys.toggle_menu = std::stoi(value);
            else if (key == "toggle_aimbot") hotkeys.toggle_aimbot = std::stoi(value);
            else if (key == "toggle_esp") hotkeys.toggle_esp = std::stoi(value);
        }
    }

    file.close();
    std::cout << "Config loaded from " << filename << std::endl;
}

void Config::ResetToDefaults() {
    // Reset overlay settings
    overlay.show_menu = true;
    overlay.vsync = true;
	overlay.show_fps = false;
    overlay.show_cursor_dot = true;
    overlay.menu_alpha = 1.0f;

    // Reset features
    features.aimbot = false;
    features.esp = false;
    features.aimbot_smoothing = 0.3f;
    features.aimbot_fov = 100.0f;
    features.aimbot_deadzone = 2;
    features.aimbot_head_targeting = true;
    features.aimbot_key = VK_RBUTTON;
    // Reset visuals
    visuals.show_health = false;
    visuals.show_enemies = false;
    visuals.show_bosses = false;
    visuals.draw_fov_circle = false;
    visuals.fov_circle_radius = 100.0f;
    visuals.crosshair = false;
    visuals.crosshair_size = 10.0f;

    // Reset colors
    colors.enemy_color[0] = 255.0f; colors.enemy_color[1] = 0.0f; colors.enemy_color[2] = 0.0f;
    colors.friendly_color[0] = 0.0f; colors.friendly_color[1] = 255.0f; colors.friendly_color[2] = 0.0f;
    colors.item_color[0] = 255.0f; colors.item_color[1] = 255.0f; colors.item_color[2] = 0.0f;
    colors.crosshair_color[0] = 255.0f; colors.crosshair_color[1] = 255.0f; colors.crosshair_color[2] = 255.0f;

    // Reset hotkeys
    hotkeys.toggle_menu = VK_INSERT;
    hotkeys.toggle_aimbot = VK_F1;
    hotkeys.toggle_esp = VK_F2;


    std::cout << "Config reset to defaults." << std::endl;
}