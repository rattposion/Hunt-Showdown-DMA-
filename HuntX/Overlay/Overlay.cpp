#include <thread>
#include "Overlay.hpp"
#include "World/World.hpp"
#include "Memory/memory.h"
#include "Offsets/Offsets.hpp"
#include "Features/Features.hpp"
static World world;
static Features features;




void drawBox(const Vector2& feet, const Vector2& head) {
    auto drawList = ImGui::GetForegroundDrawList();

    float height = feet.y - head.y;
    float width = height * 0.4f;

    float left = feet.x - (width * 0.5f);
    float right = feet.x + (width * 0.5f);

    // Draw red box
    drawList->AddRect(
        ImVec2(left, head.y),
        ImVec2(right, feet.y),
        ImColor(255, 0, 0, 255),
        0.0f, 0, 1.0f
    );
}


void drawNamedBossBox(const Vector2& feet, const Vector2& head, const char* bossName, ImU32 color) {
    auto drawList = ImGui::GetForegroundDrawList();

    float height = feet.y - head.y;
    float width = height * 0.5f; // Bosses are wider than hunters

    float left = feet.x - (width * 0.5f);
    float right = feet.x + (width * 0.5f);

    // Draw colored box for each boss type
    drawList->AddRect(
        ImVec2(left, head.y),
        ImVec2(right, feet.y),
        color,                   // Use the boss-specific color
        0.0f, 0, 3.0f           // Thick line for bosses
    );

    // Add boss name above the box
    ImVec2 textSize = ImGui::CalcTextSize(bossName);
    ImVec2 textPos = ImVec2(
        feet.x - (textSize.x * 0.5f), // Center text
        head.y - textSize.y - 5.0f    // Above the box
    );

    // Text background for readability
    drawList->AddRectFilled(
        ImVec2(textPos.x - 3, textPos.y - 2),
        ImVec2(textPos.x + textSize.x + 3, textPos.y + textSize.y + 2),
        ImColor(0, 0, 0, 200) // Dark background
    );

    // Draw the boss name with white text for readability
    drawList->AddText(textPos, ImColor(255, 255, 255, 255), bossName);

    // Optional: Add a small filled circle in the center with the boss color
    ImVec2 centerPos = ImVec2(feet.x, feet.y - (height * 0.5f));
    drawList->AddCircleFilled(centerPos, 3.0f, color);
}

// Window procedure implementation
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_SIZE:
        if (d3d_device != NULL && wParam != SIZE_MINIMIZED)
        {
            if (d3d_render_target) { d3d_render_target->Release(); d3d_render_target = NULL; }
            d3d_swap_chain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            ID3D11Texture2D* pBackBuffer;
            d3d_swap_chain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            d3d_device->CreateRenderTargetView(pBackBuffer, NULL, &d3d_render_target);
            pBackBuffer->Release();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

bool overlay::init()
{
    // Create window class
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0L;
    wc.cbWndExtra = 0L;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"OverlayClass";
    wc.hIconSm = NULL;

    RegisterClassEx(&wc);

    // Create window
    window = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        L"OverlayClass",
        L"Overlay",
        WS_POPUP,
        0, 0, monitor.width, monitor.height,
        NULL, NULL, wc.hInstance, NULL
    );

    if (!window)
        return false;

    // Set window attributes for transparency
    SetLayeredWindowAttributes(window, RGB(0, 0, 0), 255, LWA_ALPHA);

    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(window, &margins);

    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);

    return true;
}


void overlay::render()
{
    ImVec4 clear_color = ImColor(0, 0, 0, 0);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    auto& config = Config::getInstance();

    actor::loop();

    if (config.overlay.show_cursor_dot && config.overlay.show_menu)
    {
        ImGui::GetForegroundDrawList()->AddCircleFilled(ImGui::GetMousePos(), 5, ImColor(0, 247, 255));
    }

    if (config.overlay.show_menu)
    {
        ImGui::SetNextWindowBgAlpha(config.overlay.menu_alpha);
        ImGui::Begin("Game Overlay", &config.overlay.show_menu, ImGuiWindowFlags_AlwaysAutoResize);

        if (ImGui::CollapsingHeader("Features"))
        {
            ImGui::Checkbox("Aimbot", &config.features.aimbot);
            if (config.features.aimbot) {
                ImGui::Indent();
                ImGui::SliderFloat("Smoothing", &config.features.aimbot_smoothing, 0.1f, 1.0f);
                ImGui::SliderFloat("FOV", &config.features.aimbot_fov, 50.0f, 300.0f);
                ImGui::SliderInt("Deadzone", &config.features.aimbot_deadzone, 1, 10);
                ImGui::Checkbox("Head Targeting", &config.features.aimbot_head_targeting);

                // Key selector
                const char* keys[] = { "Right Mouse", "Left Mouse", "Middle Mouse", "X1 Mouse", "X2 Mouse" };
                int current_key = 0;
                if (config.features.aimbot_key == VK_RBUTTON) current_key = 0;
                else if (config.features.aimbot_key == VK_LBUTTON) current_key = 1;
                else if (config.features.aimbot_key == VK_MBUTTON) current_key = 2;
                else if (config.features.aimbot_key == VK_XBUTTON1) current_key = 3;
                else if (config.features.aimbot_key == VK_XBUTTON2) current_key = 4;

                if (ImGui::Combo("Aimbot Key", &current_key, keys, 5)) {
                    switch (current_key) {
                    case 0: config.features.aimbot_key = VK_RBUTTON; break;
                    case 1: config.features.aimbot_key = VK_LBUTTON; break;
                    case 2: config.features.aimbot_key = VK_MBUTTON; break;
                    case 3: config.features.aimbot_key = VK_XBUTTON1; break;
                    case 4: config.features.aimbot_key = VK_XBUTTON2; break;
                    }
                }
                ImGui::Unindent();
            }
            ImGui::Checkbox("ESP", &config.features.esp);
        }

        if (ImGui::CollapsingHeader("Visuals"))
        {
            ImGui::Checkbox("Show Health", &config.visuals.show_health);
            ImGui::Checkbox("Show Enemies", &config.visuals.show_enemies);
            ImGui::Checkbox("Show Bosses", &config.visuals.show_bosses);
            ImGui::Checkbox("FOV Circle", &config.visuals.draw_fov_circle);
            if (config.visuals.draw_fov_circle)
            {
                ImGui::SliderFloat("FOV Radius", &config.features.aimbot_fov, 50.0f, 300.0f);
            }
            ImGui::Checkbox("Crosshair", &config.visuals.crosshair);
            if (config.visuals.crosshair)
            {
                ImGui::SliderFloat("Crosshair Size", &config.visuals.crosshair_size, 5.0f, 50.0f);
            }
        }

        if (ImGui::CollapsingHeader("Colors"))
        {
            ImGui::ColorEdit3("Enemy Color", config.colors.enemy_color);
            ImGui::ColorEdit3("Friendly Color", config.colors.friendly_color);
            ImGui::ColorEdit3("Item Color", config.colors.item_color);
            ImGui::ColorEdit3("Crosshair Color", config.colors.crosshair_color);
        }

        if (ImGui::CollapsingHeader("Settings"))
        {
            ImGui::Checkbox("VSync", &config.overlay.vsync);
            ImGui::Checkbox("Show FPS", &config.overlay.show_fps);
            ImGui::Checkbox("Show Cursor Dot", &config.overlay.show_cursor_dot);
            ImGui::SliderFloat("Menu Alpha", &config.overlay.menu_alpha, 0.1f, 1.0f);

            if (ImGui::Button("Save Config"))
            {
                config.SaveConfig();
            }
            ImGui::SameLine();
            if (ImGui::Button("Load Config"))
            {
                config.LoadConfig();
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset to Defaults"))
            {
                config.ResetToDefaults();
            }
        }

        // Display current feature status
        ImGui::Separator();
        ImGui::Text("Active Features:");
        if (config.features.aimbot) ImGui::TextColored(ImVec4(0, 1, 0, 1), "• Aimbot");
        if (config.features.esp) ImGui::TextColored(ImVec4(0, 1, 0, 1), "• ESP");

        ImGui::End();
    }

    // Draw crosshair if enabled
    if (config.visuals.crosshair)
    {
        ImVec2 center = ImVec2(monitor.width / 2.0f, monitor.height / 2.0f);
        ImU32 crosshair_color = ImColor(
            config.colors.crosshair_color[0] / 255.0f,
            config.colors.crosshair_color[1] / 255.0f,
            config.colors.crosshair_color[2] / 255.0f
        );
        float size = config.visuals.crosshair_size;

        ImGui::GetForegroundDrawList()->AddLine(
            ImVec2(center.x - size, center.y),
            ImVec2(center.x + size, center.y),
            crosshair_color, 2.0f
        );
        ImGui::GetForegroundDrawList()->AddLine(
            ImVec2(center.x, center.y - size),
            ImVec2(center.x, center.y + size),
            crosshair_color, 2.0f
        );
    }

    // Draw FOV circle if enabled
    if (config.visuals.draw_fov_circle)
    {
        ImVec2 center = ImVec2(monitor.width / 2.0f, monitor.height / 2.0f);
        ImGui::GetForegroundDrawList()->AddCircle(center, config.visuals.fov_circle_radius, ImColor(255, 255, 255, 100), 0, 2.0f);
    }

    // Update Camera for each frame
    world.UpdateCam();


    // *** CRITICAL: Call aimbot logic every frame ***
    features.UpdateAimbot();

    // Draw aimbot target indicator (use config key instead of hardcoded VK_RBUTTON)
    if (config.features.aimbot && (GetAsyncKeyState(config.features.aimbot_key) & 0x8000)) {
        Vector2 target = features.GetClosestEnemyInFOV();
        if (target != Vector2::Zero()) {
            // Draw a small circle around the target
            ImGui::GetForegroundDrawList()->AddCircle(
                ImVec2(target.x, target.y),
                10.0f,
                ImColor(255, 0, 0, 255), // Red circle
                12,
                2.0f
            );

            // Optional: Draw crosshair on target
            ImGui::GetForegroundDrawList()->AddLine(
                ImVec2(target.x - 5, target.y),
                ImVec2(target.x + 5, target.y),
                ImColor(255, 0, 0, 255), 2.0f
            );
            ImGui::GetForegroundDrawList()->AddLine(
                ImVec2(target.x, target.y - 5),
                ImVec2(target.x, target.y + 5),
                ImColor(255, 0, 0, 255), 2.0f
            );
        }
    }

    if (config.features.esp)
    {
        if (config.visuals.show_enemies)
        {
            // Get current entity buffer
            int readBuffer = currentBufferIndex.load();
            const EntityBuffer& entities = entityBuffers[readBuffer];

            if (!entities.hunter.empty()) {
                std::vector<Vector3> positions(entities.hunter.size());

                auto scatterHandle = mem.CreateScatterHandle();
                for (size_t i = 0; i < entities.hunter.size(); ++i) {
                    mem.AddScatterReadRequest(scatterHandle, entities.hunter[i] + Offsets::Position, &positions[i], sizeof(Vector3));
                }
                mem.ExecuteReadScatter(scatterHandle);
                mem.CloseScatterHandle(scatterHandle);

                // Store head positions for aimbot to use
                std::vector<Vector3> headWorldPositions;
                std::vector<Vector2> headScreenPositions;

                for (size_t i = 0; i < entities.hunter.size(); ++i) {
                    Vector3 feetPos = positions[i];
                    Vector3 MaxPos = Vector3(feetPos.x, feetPos.y, feetPos.z + 2.0f);
                    Vector3 HeadPos = Vector3(feetPos.x, feetPos.y, feetPos.z + 1.7f);

                    Vector2 feetScreen = world.W2s(feetPos);
                    Vector2 MaxScreen = world.W2s(MaxPos);
                    Vector2 HeadScreen = world.W2s(HeadPos);

                    // Store for aimbot
                    headWorldPositions.push_back(HeadPos);
                    headScreenPositions.push_back(HeadScreen);

                    if (feetScreen != Vector2::Zero() && MaxScreen != Vector2::Zero()) {
                        drawBox(feetScreen, MaxScreen);

                        // Optional: Draw a small dot at head position to verify
                        if (HeadScreen != Vector2::Zero()) {
                            ImGui::GetForegroundDrawList()->AddCircleFilled(
                                ImVec2(HeadScreen.x, HeadScreen.y),
                                3.0f,
                                ImColor(0, 255, 0, 255) // Green dot at head position
                            );
                        }
                    }
                }

                // Share position data with aimbot (thread-safe)
                {
                    std::lock_guard<std::mutex> lock(Features::positionDataMutex);
                    Features::lastHeadWorldPositions = headWorldPositions;
                    Features::lastHeadScreenPositions = headScreenPositions;
                }
            }
        }

        if (config.visuals.show_bosses)
        {
            int readBuffer = currentBufferIndex.load();
            const EntityBuffer& entities = entityBuffers[readBuffer];

            std::vector<BossEntity> allBosses;

            // Add all boss types
            for (const auto& entity : entities.immolator) {
                allBosses.push_back({ entity, "Immolator", ImColor(255, 100, 0, 255) }); // Orange
            }
            for (const auto& entity : entities.butcher) {
                allBosses.push_back({ entity, "Butcher", ImColor(150, 0, 150, 255) }); // Purple
            }
            for (const auto& entity : entities.spider) {
                allBosses.push_back({ entity, "Spider", ImColor(100, 255, 100, 255) }); // Light green
            }
            for (const auto& entity : entities.scrapbeak) {
                allBosses.push_back({ entity, "Scrapbeak", ImColor(255, 255, 100, 255) }); // Yellow
            }
            for (const auto& entity : entities.assassin) {
                allBosses.push_back({ entity, "Assassin", ImColor(0, 255, 255, 255) }); // Cyan
            }
            for (const auto& entity : entities.rotjaw) {
                allBosses.push_back({ entity, "Rotjaw", ImColor(255, 0, 255, 255) }); // Magenta
            }

            if (!allBosses.empty()) {
                // Read all boss positions at once with scatter read
                std::vector<Vector3> positions(allBosses.size());
                auto scatterHandle = mem.CreateScatterHandle();
                for (size_t i = 0; i < allBosses.size(); ++i) {
                    mem.AddScatterReadRequest(scatterHandle, allBosses[i].entityBase + Offsets::Position, &positions[i], sizeof(Vector3));
                }
                mem.ExecuteReadScatter(scatterHandle);
                mem.CloseScatterHandle(scatterHandle);

                // Draw all bosses with their specific names and colors
                for (size_t i = 0; i < allBosses.size(); ++i) {
                    Vector3 feetPos = positions[i];
                    Vector3 headPos = Vector3(feetPos.x, feetPos.y, feetPos.z + 2.5f); // Bosses are taller

                    Vector2 feetScreen = world.W2s(feetPos);
                    Vector2 headScreen = world.W2s(headPos);

                    if (feetScreen != Vector2::Zero() && headScreen != Vector2::Zero()) {
                        drawNamedBossBox(feetScreen, headScreen, allBosses[i].name, allBosses[i].color);
                    }
                }
            }
        }
    }

    // *** MOVED OUTSIDE ESP BLOCK - This must be called every frame ***
    ImGui::Render();

    const float clear_color_with_alpha[4] = {
        clear_color.x * clear_color.w,
        clear_color.y * clear_color.w,
        clear_color.z * clear_color.w,
        clear_color.w
    };

    d3d_device_ctx->OMSetRenderTargets(1, &d3d_render_target, nullptr);
    d3d_device_ctx->ClearRenderTargetView(d3d_render_target, clear_color_with_alpha);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    d3d_swap_chain->Present(config.overlay.vsync ? 1 : 0, 0);
}

bool overlay::render_loop()
{
    MSG msg = { NULL };
    ZeroMemory(&msg, sizeof(MSG));

    static int topmost_counter = 0;


    while (msg.message != WM_QUIT)
    {

        
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        // Handle input
        ImGuiIO& io = ImGui::GetIO();
        io.DeltaTime = 1.0f / 60.0f;

        POINT p_cursor;
        GetCursorPos(&p_cursor);
        ScreenToClient(window, &p_cursor);
        io.MousePos.x = (float)p_cursor.x;
        io.MousePos.y = (float)p_cursor.y;

        io.MouseDown[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        io.MouseDown[1] = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        io.MouseDown[2] = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;

        // Toggle menu with INSERT key
        if (GetAsyncKeyState(VK_INSERT) & 1)
            Config::getInstance().overlay.show_menu = !Config::getInstance().overlay.show_menu;

        if (++topmost_counter >= 120) {
            // Get the current foreground window
            HWND foregroundWindow = GetForegroundWindow();

            // Only reposition if we're not already the foreground window
            if (foregroundWindow != window) {
                // Method 1: Use SetWindowPos with the current foreground window as reference
                SetWindowPos(window, foregroundWindow, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

                // Method 2: Alternative - bring to front manually
                BringWindowToTop(window);

                // Method 3: Set as foreground (more aggressive)
                // SetForegroundWindow(window);
            }
            topmost_counter = 0;
        }

        overlay::render();
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (d3d_render_target) { d3d_render_target->Release(); d3d_render_target = NULL; }
    if (d3d_swap_chain) { d3d_swap_chain->Release(); d3d_swap_chain = NULL; }
    if (d3d_device_ctx) { d3d_device_ctx->Release(); d3d_device_ctx = NULL; }
    if (d3d_device) { d3d_device->Release(); d3d_device = NULL; }

    DestroyWindow(window);
    UnregisterClass(L"OverlayClass", GetModuleHandle(NULL));

    return true;
}

bool overlay::init_dx11()
{
    DXGI_SWAP_CHAIN_DESC swap_chain_description = {};
    swap_chain_description.BufferCount = 2;
    swap_chain_description.BufferDesc.Width = 0;
    swap_chain_description.BufferDesc.Height = 0;
    swap_chain_description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_description.BufferDesc.RefreshRate.Numerator = 60;
    swap_chain_description.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_description.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    swap_chain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_description.OutputWindow = window;
    swap_chain_description.SampleDesc.Count = 1;
    swap_chain_description.SampleDesc.Quality = 0;
    swap_chain_description.Windowed = TRUE;
    swap_chain_description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL d3d_feature_lvl;
    const D3D_FEATURE_LEVEL d3d_feature_array[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

    HRESULT result = D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        0,
        d3d_feature_array,
        2,
        D3D11_SDK_VERSION,
        &swap_chain_description,
        &d3d_swap_chain,
        &d3d_device,
        &d3d_feature_lvl,
        &d3d_device_ctx
    );

    if (FAILED(result))
        return false;

    ID3D11Texture2D* pBackBuffer;
    d3d_swap_chain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    d3d_device->CreateRenderTargetView(pBackBuffer, NULL, &d3d_render_target);
    pBackBuffer->Release();

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup ImGui style
    ImGui::StyleColorsDark();

    ImFontConfig font_config;
    font_config.PixelSnapH = false;
    font_config.OversampleH = 5;
    font_config.OversampleV = 5;
    font_config.RasterizerMultiply = 1.2f;

    // Initialize ImGui backends
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(d3d_device, d3d_device_ctx);

    return true;
}