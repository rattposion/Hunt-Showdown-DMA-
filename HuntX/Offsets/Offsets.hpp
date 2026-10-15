#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <atomic>
#include <mutex>

namespace Offsets
{
    // Global systems
    constexpr uint64_t SSystemGlobalEnvironment = 0x2991318;
    constexpr uint64_t EntitySystem = 0xC0;
    constexpr uint64_t PSystem = 0x90;

    // Entity list & tracking
    constexpr uint64_t EntityList = 0x40078;
    constexpr uint64_t EntityArray = 0x400A0;
    constexpr uint64_t ObjectCount = 0x40092;

    // Entity-specific
    constexpr uint64_t Position = 0x134;

    // Camera offsets
    constexpr uint64_t CameraBaseOffset = 0x838;
    constexpr uint64_t ViewMatrixOffset = 0x230;
    constexpr uint64_t CameraPosOffset = 0x2F0;
    constexpr uint64_t ProjectionMatrixOffset = 0x270;

    // Localplayer Offsets
    constexpr uint64_t IGameFrameworkOffset = 0xD8;
    constexpr uint64_t pGame = 0x700;
    constexpr uint64_t pGameClientNubOffset = 0x48;
    constexpr uint64_t pGameClientChannelOffset = 0x20;
    constexpr uint64_t localPlayerIdxOffset = 0x24;
}

// Declare global variables as extern (declaration only)
extern uintptr_t base;
extern uintptr_t size;
extern uint64_t CameraBase;
extern uint64_t SSystemGlobalEnvironment;
extern uint64_t EntitySystem;
extern uint64_t PSystem;
extern uint64_t IGameFramework;
extern uint16_t NumberOfObjects;
extern uint64_t EntityListAddress;
extern uint64_t entityBase;
extern uintptr_t entityClassPtr;


struct entityName {
    char name[100];
};



// Double buffer structure for thread-safe entity access
struct EntityBuffer {
    std::vector<uint64_t> hunter, immolator, butcher, spider, scrapbeak, assassin, rotjaw;

    void clear() {
        hunter.clear();
        immolator.clear();
        butcher.clear();
        spider.clear();
        scrapbeak.clear();
        assassin.clear();
        rotjaw.clear();
    }
};

// Double buffer globals
extern EntityBuffer entityBuffers[2];  // Two buffers
extern std::atomic<int> currentBufferIndex;  // Which buffer readers should use
extern std::mutex bufferSwapMutex;  // Protects buffer swapping