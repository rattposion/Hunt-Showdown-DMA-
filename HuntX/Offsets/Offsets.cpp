#include "Offsets.hpp"
#include <mutex>



// Define the global variables (definition)
uintptr_t base = 0;
uintptr_t size = 0;
uint64_t EntityList = 0;
uint64_t CameraBase = 0;

uint64_t SSystemGlobalEnvironment = 0;
uint64_t EntitySystem = 0;
uint64_t PSystem = 0;
uint64_t IGameFramework = 0;
uint16_t NumberOfObjects = 0;
uint64_t EntityListAddress = 0;
uint64_t entityBase = 0;
uintptr_t entityClassPtr = 0;


// Define the double buffer storage
EntityBuffer entityBuffers[2];
std::atomic<int> currentBufferIndex{ 0 };
std::mutex bufferSwapMutex;