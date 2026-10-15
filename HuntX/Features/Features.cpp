#include "Features.hpp"
#include "World/World.hpp"
#include "Memory/Memory.h"
#include "Offsets/Offsets.hpp"
#include "Config/Config.hpp"
#include <setupapi.h>
#include <devguid.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <mutex>

#pragma comment(lib, "setupapi.lib")

// Define static members
std::vector<Vector2> Features::lastHeadScreenPositions;
std::vector<Vector3> Features::lastHeadWorldPositions;
std::mutex Features::positionDataMutex;

// Fix the constructors
KMBox::KMBox() : hSerial(INVALID_HANDLE_VALUE) {}
KMBox::~KMBox() { disconnect(); }

Features::Features() : aimbotActive(false), lastAimTarget(Vector2::Zero()) {
    // Test KMBox connection with multiple ports
    bool kmboxConnected = false;

    std::vector<std::string> comPorts = { "COM3", "COM4", "COM5", "COM6", "COM7", "COM8" };

    for (const auto& port : comPorts) {
        if (kmbox.openSerialPort(port, 115200)) {
            std::cout << "KMBox connected successfully on " << port << "!" << std::endl;
            kmboxConnected = true;

            // *** TEST THE CONNECTION ***
            std::cout << "Testing KMBox movement..." << std::endl;
            kmbox.move(5, 5);   // Small test movement
            kmbox.move(-5, -5); // Move back
            std::cout << "KMBox test movement completed!" << std::endl;
            break;
        }
    }

    if (!kmboxConnected) {
        std::cout << "*** WARNING: Failed to connect to KMBox on any COM port! ***" << std::endl;
        std::cout << "*** Aimbot will NOT work without KMBox connection! ***" << std::endl;
    }
}

Features::~Features() {}

// Your existing KMBox methods...
std::string KMBox::getCOMPortByDescription(const std::string& targetDescription) {
    HDEVINFO hDevInfo = SetupDiGetClassDevsA(&GUID_DEVCLASS_PORTS, 0, 0, DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE) return "";

    SP_DEVINFO_DATA deviceInfoData;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &deviceInfoData); ++i) {
        char buf[512];
        DWORD nSize = 0;

        if (SetupDiGetDeviceRegistryPropertyA(
            hDevInfo, &deviceInfoData, SPDRP_FRIENDLYNAME, NULL,
            (PBYTE)buf, sizeof(buf), &nSize) && nSize > 0)
        {
            buf[nSize] = '\0';
            std::string deviceDescription = buf;

            size_t comPos = deviceDescription.find("COM");
            size_t endPos = deviceDescription.find(")", comPos);

            if (comPos != std::string::npos && endPos != std::string::npos &&
                deviceDescription.find(targetDescription) != std::string::npos)
            {
                SetupDiDestroyDeviceInfoList(hDevInfo);
                return deviceDescription.substr(comPos, endPos - comPos);
            }
        }
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    return "";
}

bool KMBox::openSerialPort(const std::string& portName, DWORD baudRate) {
    std::string fullPort = "\\\\.\\" + portName;

    hSerial = CreateFileA(fullPort.c_str(), GENERIC_READ | GENERIC_WRITE,
        0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (hSerial == INVALID_HANDLE_VALUE) return false;

    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(dcb);

    if (!GetCommState(hSerial, &dcb)) { CloseHandle(hSerial); return false; }

    dcb.BaudRate = baudRate;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcb)) { CloseHandle(hSerial); return false; }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 5;
    timeouts.ReadTotalTimeoutConstant = 5;
    timeouts.WriteTotalTimeoutConstant = 5;
    SetCommTimeouts(hSerial, &timeouts);

    return true;
}

bool KMBox::connect(const std::string& deviceDescription, DWORD baud) {
    std::string port = getCOMPortByDescription(deviceDescription);
    if (port.empty()) return false;
    return openSerialPort(port, baud);
}

void KMBox::disconnect() {
    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
    }
}

void KMBox::send(const std::string& cmd) {
    if (hSerial == INVALID_HANDLE_VALUE) return;
    DWORD written;
    WriteFile(hSerial, cmd.c_str(), (DWORD)cmd.length(), &written, NULL);
}

bool KMBox::read(std::string& response) {
    if (hSerial == INVALID_HANDLE_VALUE) return false;
    char buf[256];
    DWORD read;
    if (!ReadFile(hSerial, buf, sizeof(buf) - 1, &read, NULL)) return false;
    if (read > 0) {
        buf[read] = '\0';
        response = buf;
        return true;
    }
    return false;
}

void KMBox::move(int x, int y) {
    send("km.move(" + std::to_string(x) + "," + std::to_string(y) + ")\r\n");
}

// New aimbot methods
bool Features::IsWithinFOV(const Vector2& screenPos, float fovRadius) {
    // Get screen center
    Vector2 screenCenter(1920 / 2.0f, 1080 / 2.0f); // Use your actual screen resolution

    // Calculate distance from center
    float dx = screenPos.x - screenCenter.x;
    float dy = screenPos.y - screenCenter.y;
    float distance = sqrt(dx * dx + dy * dy);

    return distance <= fovRadius;
}

Vector2 Features::GetClosestEnemyInFOV() {
    auto& config = Config::getInstance();

    // Use the shared position data instead of doing another memory read
    std::vector<Vector2> headScreenPositions;
    {
        std::lock_guard<std::mutex> lock(positionDataMutex);
        headScreenPositions = lastHeadScreenPositions;
    }

    if (headScreenPositions.empty()) {
        return Vector2::Zero();
    }

    // Only log occasionally to reduce spam
    static int logCounter = 0;
    bool shouldLog = (++logCounter % 60 == 0); // Log every 60 calls

    if (shouldLog) {
        std::cout << "[AIMBOT] Checking " << headScreenPositions.size() << " head positions" << std::endl;
    }

    // Find closest enemy in FOV
    Vector2 screenCenter(1920 / 2.0f, 1080 / 2.0f);
    Vector2 closestTarget = Vector2::Zero();
    float closestDistance = FLT_MAX;
    int validTargets = 0;

    for (size_t i = 0; i < headScreenPositions.size(); ++i) {
        Vector2 HeadScreen = headScreenPositions[i];

        // Check if valid screen coordinates
        if (HeadScreen == Vector2::Zero()) {
            continue;
        }

        // Check if within FOV (use config FOV)
        if (!IsWithinFOV(HeadScreen, config.features.aimbot_fov)) {
            continue;
        }

        validTargets++;

        // Calculate distance from crosshair
        float dx = HeadScreen.x - screenCenter.x;
        float dy = HeadScreen.y - screenCenter.y;
        float distance = sqrt(dx * dx + dy * dy);

        // Update closest target
        if (distance < closestDistance) {
            closestDistance = distance;
            closestTarget = HeadScreen;
        }
    }

    if (shouldLog && validTargets > 0) {
        std::cout << "[AIMBOT] Found " << validTargets << " valid targets in FOV" << std::endl;
        if (closestTarget != Vector2::Zero()) {
            std::cout << "[AIMBOT] Closest target at distance: " << closestDistance << std::endl;
        }
    }

    return closestTarget;
}

void Features::AimbotLogic() {
    auto& config = Config::getInstance();

    // Check if aimbot is enabled in config
    if (!config.features.aimbot) {
        return;
    }

    // Check if aimbot key is held down (configurable)
    if (!(GetAsyncKeyState(config.features.aimbot_key) & 0x8000)) {
        return;
    }

    // Get closest enemy in FOV
    Vector2 target = GetClosestEnemyInFOV();

    if (target == Vector2::Zero()) {
        // Only log occasionally to reduce spam
        static int noTargetCounter = 0;
        if (++noTargetCounter % 30 == 0) {
            std::cout << "[AIMBOT] No targets found in FOV" << std::endl;
        }
        return;
    }

    std::cout << "[AIMBOT] TARGET ACQUIRED at screen pos: " << target.x << ", " << target.y << std::endl;

    // Calculate movement needed
    Vector2 screenCenter(1920 / 2.0f, 1080 / 2.0f);
    float deltaX = target.x - screenCenter.x;
    float deltaY = target.y - screenCenter.y;

    std::cout << "[AIMBOT] Raw delta: " << deltaX << ", " << deltaY << std::endl;

    // Apply smoothing from config
    deltaX = deltaX * config.features.aimbot_smoothing;
    deltaY = deltaY * config.features.aimbot_smoothing;

    std::cout << "[AIMBOT] Smoothed delta: " << deltaX << ", " << deltaY << std::endl;

    // Convert to integers for mouse movement
    int moveX = (int)round(deltaX);
    int moveY = (int)round(deltaY);

    // Only move if the movement is significant enough (using config deadzone)
    //if (abs(moveX) > config.features.aimbot_deadzone || abs(moveY) > config.features.aimbot_deadzone) {
        std::cout << "[AIMBOT] *** MOVING MOUSE: " << moveX << ", " << moveY << " ***" << std::endl;

        // *** CRITICAL FIX: Make sure KMBox is properly initialized ***
        try {
            kmbox.move(moveX, moveY);
            std::cout << "[AIMBOT] Mouse command sent successfully!" << std::endl;
        }
        catch (...) {
            std::cout << "[AIMBOT] ERROR: Failed to send mouse command!" << std::endl;
        }
   // }
    //else {
    //    std::cout << "[AIMBOT] Movement too small (deadzone: " << config.features.aimbot_deadzone << ")" << std::endl;
    //}
}

void Features::UpdateAimbot() {
    AimbotLogic();
}

void Features::Kmbox() {
    // Placeholder method - you can implement additional KMBox functionality here
}