#include "Entitylist.hpp"
#include "Memory/Memory.h"
#include "Offsets/Offsets.hpp"  
#include <iostream>
#include <vector>
#include <mutex>

void Entitylist::Init()
{
    if (!mem.Init("HuntGame.exe", true, true)) {
        std::cout << "[SCANNER] Failed to initialize DMA for process: " << std::endl;
    }


    if (!mem.FixCr3()) {
        std::cout << "Failed to fix CR3" << std::endl;
    }

    base = mem.GetBaseDaddy("GameHunt.dll");

    auto handle = mem.CreateScatterHandle();

    mem.AddScatterReadRequest(handle, base + Offsets::SSystemGlobalEnvironment, &SSystemGlobalEnvironment, sizeof(uint64_t));
    mem.ExecuteReadScatter(handle);
    std::cout << "SSystemGlobalEnvironment: " << std::hex << SSystemGlobalEnvironment << std::endl;


    mem.AddScatterReadRequest(handle, SSystemGlobalEnvironment + Offsets::EntitySystem, &EntitySystem, sizeof(uint64_t));
	mem.AddScatterReadRequest(handle, SSystemGlobalEnvironment + Offsets::PSystem, &PSystem, sizeof(uint64_t));
    mem.ExecuteReadScatter(handle);

    std::cout << "EntitySystem: " << std::hex << EntitySystem << std::endl;
    std::cout << "PSystem: " << std::hex << PSystem << std::endl;


    mem.AddScatterReadRequest(handle, PSystem + Offsets::CameraBaseOffset, &CameraBase, sizeof(uint64_t));
    mem.ExecuteReadScatter(handle);
	std::cout << "CameraBase: " << std::hex << CameraBase << std::endl;

    EntityListAddress = EntitySystem + Offsets::EntityList;

	mem.CloseScatterHandle(handle);
}

void Entitylist::run() {
    /*
	 // Enable for testing if no dma card plugged in
    while (true) {
		std::cout << "[SCANNER] Waiting for game to start..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}*/


	Entitylist::Init();

    while (true) {
        const uint32_t BATCH_SIZE = 500;
        uint64_t entityBases[BATCH_SIZE];
        uint64_t entityClassPtrs[BATCH_SIZE];
        uint64_t entityClassPtrs2[BATCH_SIZE];
        entityName entityClassNames[BATCH_SIZE];

        // Write to the non-active buffer (1 - currentBufferIndex)
        int writeBuffer = 1 - currentBufferIndex.load();

        entityBuffers[writeBuffer].clear();

        auto Entitylisthandle = mem.CreateScatterHandle();

        mem.AddScatterReadRequest(Entitylisthandle, EntitySystem + Offsets::ObjectCount, &NumberOfObjects, sizeof(uint16_t));
        mem.ExecuteReadScatter(Entitylisthandle);

        if (NumberOfObjects == 0)
            std::cout << "[SCANNER] No entities found in the game. Output: " << NumberOfObjects << std::endl;

        for (uint32_t batchStart = 0; batchStart < NumberOfObjects; batchStart += BATCH_SIZE) {
            uint32_t currentBatchSize = min(BATCH_SIZE, NumberOfObjects - batchStart);

            // Batch 1: Read entity bases
            for (uint32_t i = 0; i < currentBatchSize; ++i) {
                uint32_t index = batchStart + i;
                mem.AddScatterReadRequest(Entitylisthandle, EntityListAddress + index * sizeof(uint64_t), &entityBases[i], sizeof(uint64_t));
            }
            mem.ExecuteReadScatter(Entitylisthandle);


            // Batch 2: Read first class pointers (only for valid entities)
            uint32_t validCount = 0;
            uint32_t validIndices[BATCH_SIZE];

            for (uint32_t i = 0; i < currentBatchSize; ++i) {
                if (entityBases[i] != 0) {
                    mem.AddScatterReadRequest(Entitylisthandle, entityBases[i] + 0x18, &entityClassPtrs[validCount], sizeof(uint64_t));
                    validIndices[validCount] = i;
                    validCount++;
                }
            }
            if (validCount == 0) continue;
            mem.ExecuteReadScatter(Entitylisthandle);


            // Batch 3: Read second class pointers
            for (uint32_t i = 0; i < validCount; ++i) {
                mem.AddScatterReadRequest(Entitylisthandle, entityClassPtrs[i] + 0x10, &entityClassPtrs2[i], sizeof(uint64_t));
            }
            mem.ExecuteReadScatter(Entitylisthandle);

            // Batch 4: Read class names
            for (uint32_t i = 0; i < validCount; ++i) {
                mem.AddScatterReadRequest(Entitylisthandle, entityClassPtrs2[i], &entityClassNames[i], sizeof(entityName));
            }
            mem.ExecuteReadScatter(Entitylisthandle);

            // Process results - simple string checks
            for (uint32_t i = 0; i < validCount; ++i) {
                entityClassNames[i].name[99] = '\0'; // Safety null terminator
                char* ClassName = entityClassNames[i].name;


                if (strstr(ClassName, "HunterBasic") != NULL) {
                    entityBuffers[writeBuffer].hunter.push_back(entityBases[validIndices[i]]);
					//std::cout << "[ found hunter ] " << entityBases[validIndices[i]] << std::endl;
                }
                else if (strstr(ClassName, "immolator_elite") != NULL) {
                    entityBuffers[writeBuffer].immolator.push_back(entityBases[validIndices[i]]);
                }
                else if (strstr(ClassName, "target_butcher") != NULL) {
                    entityBuffers[writeBuffer].butcher.push_back(entityBases[validIndices[i]]);
                }
                else if (strstr(ClassName, "target_spider") != NULL) {
                    entityBuffers[writeBuffer].spider.push_back(entityBases[validIndices[i]]);
                }
                else if (strstr(ClassName, "target_scrapbeak") != NULL) {
                    entityBuffers[writeBuffer].scrapbeak.push_back(entityBases[validIndices[i]]);
                }
                else if (strstr(ClassName, "target_assassin") != NULL) {
                    entityBuffers[writeBuffer].assassin.push_back(entityBases[validIndices[i]]);
                }
                else if (strstr(ClassName, "target_rotjaw") != NULL) {
                    entityBuffers[writeBuffer].rotjaw.push_back(entityBases[validIndices[i]]);
                }
            }
        }

        mem.CloseScatterHandle(Entitylisthandle);
        // Swap buffers
        std::lock_guard<std::mutex> lock(bufferSwapMutex);
        currentBufferIndex.store(writeBuffer);
    }


}



Entitylist::Entitylist()
{
    
}

Entitylist::~Entitylist()
{
}