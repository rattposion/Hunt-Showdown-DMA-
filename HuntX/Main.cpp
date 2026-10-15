#pragma warning(push, 0)

#include <iostream>
#include <thread>


#include "Overlay/Overlay.hpp"
#include "Config/Config.hpp"
#include "Memory/memory.h"
#include "../DMALibrary/libs/leechcore.h"
#include "Entitylist/Entitylist.hpp"

#pragma comment(lib, "leechcore.lib")
#pragma comment(lib, "vmm.lib")
#pragma warning(pop)

std::atomic<bool> running(true);



int main() {
    

    std::cout << "Initializing overlay..." << std::endl;

    // Load config on startup
    Config::getInstance().LoadConfig();

    // Start render thread
    std::thread render_thread([]() {
        // Initialize the overlay window ON THIS THREAD
        if (!overlay::init()) {
            std::cerr << "Failed to initialize overlay window!" << std::endl;
            running = false;
            return;
        }

        std::cout << "Overlay window created successfully!" << std::endl;

        // Initialize DirectX 11 ON THIS THREAD
        if (!overlay::init_dx11()) {
            std::cerr << "Failed to initialize DirectX 11!" << std::endl;
            running = false;
            return;
        }

        std::cout << "DirectX 11 initialized successfully!" << std::endl;
        std::cout << "Starting overlay render loop..." << std::endl;
        std::cout << "Press INSERT to toggle the menu!" << std::endl;

        // Run render loop
        overlay::render_loop();
        running = false;
        });

    Entitylist entityList;

    // Start entity thread
    std::thread entity_thread(&Entitylist::run, &entityList);

    // Main thread just waits
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Overlay closed." << std::endl;

    // Join threads
    render_thread.join();
    entity_thread.detach();

    // Save config on exit
    Config::getInstance().SaveConfig();

    return 0;
}