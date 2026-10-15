#pragma once
#include <string>
#include <windows.h>

class Config {
public:
    // Singleton pattern - ensures only one config instance
    static Config& getInstance();

    // Delete copy constructor and assignment operator
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    // Overlay settings
    struct Overlay {
        bool show_menu = true;
        bool vsync = true;
        bool show_fps = false;
        bool show_cursor_dot = true;
        float menu_alpha = 1.0f;
    } overlay;

    // Feature toggles - add your features here
    struct Features {
        bool aimbot = false;
        bool esp = false;

        float aimbot_smoothing = 0.3f;
        float aimbot_fov = 100.0f;
        int aimbot_deadzone = 2;
        bool aimbot_head_targeting = true;
        int aimbot_key = VK_RBUTTON;
    } features;

    // Visual settings
    struct Visuals {
        bool show_health = false;
        bool show_enemies = false;
        bool show_bosses = false;
        bool draw_fov_circle = false;
        float fov_circle_radius = 100.0f;
        bool crosshair = false;
        float crosshair_size = 10.0f;
    } visuals;

    // Colors (RGB values 0-255)
    struct Colors {
        float enemy_color[3] = { 255.0f, 0.0f, 0.0f };      // Red
        float friendly_color[3] = { 0.0f, 255.0f, 0.0f };   // Green
        float item_color[3] = { 255.0f, 255.0f, 0.0f };     // Yellow
        float crosshair_color[3] = { 255.0f, 255.0f, 255.0f }; // White
    } colors;

    // Hotkeys
    struct Hotkeys {
        int toggle_menu = VK_INSERT;
        int toggle_aimbot = VK_F1;
        int toggle_esp = VK_F2;
    } hotkeys;


    // Methods to save/load config
    void SaveConfig(const std::string& filename = "config.ini");
    void LoadConfig(const std::string& filename = "config.ini");
    void ResetToDefaults();

private:
    Config() = default; // Private constructor for singleton
    ~Config() = default;
};