#pragma once
#include <utility>
#include <cstddef>
#include <intrin.h>
#include <string>
#include <vector>  // Add this include
#include <cstdint> // Add this include
#include <d3d11.h>
#include <dwmapi.h>
#include <iostream>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <windows.h>
#include "../Config/Config.hpp"
#include "Vector/Vector.hpp"

// Link required libraries automatically
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwmapi.lib")

// Forward declarations
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Global variables
inline HWND window;
inline ID3D11Device* d3d_device;
inline ID3D11DeviceContext* d3d_device_ctx;
inline IDXGISwapChain* d3d_swap_chain;
inline ID3D11RenderTargetView* d3d_render_target;

// Simple monitor structure
struct Monitor {
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
};
inline Monitor monitor;

// Simple string encryption class (placeholder)
class EncryptedString {
private:
    std::string str;
public:
    EncryptedString(const char* s) : str(s) {}
    std::string decrypt() { return str; }
};

// Macros and helper functions
#define hide // Empty macro for now
#define hide_call(func) func
#define enc(str) EncryptedString(str)

// Actor namespace (placeholder)
namespace actor {
    inline void loop() {
        // Placeholder for actor loop functionality
        // You can add your game-specific logic here
    }
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void drawBox(const Vector2& feet, const Vector2& head);
void drawNamedBossBox(const Vector2& feet, const Vector2& head, const char* bossName, ImU32 color);


class overlay
{
public:
    static bool init();
    static void render();
    static bool render_loop();
    static bool init_dx11();
};


struct BossEntity {
    uint64_t entityBase;
    const char* name;
    ImU32 color;
};