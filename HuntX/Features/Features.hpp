#pragma once
#include <windows.h>
#include <string>
#include <vector>  // Added this include
#include <mutex>   // Added this include
#include "Vector/Vector.hpp"

class KMBox
{
public:
    KMBox();
    ~KMBox();

    std::string getCOMPortByDescription(const std::string& targetDescription);
    bool openSerialPort(const std::string& portName, DWORD baudRate);
    bool connect(const std::string& deviceDescription, DWORD baud);
    void disconnect();
    void send(const std::string& cmd);
    bool read(std::string& response);
    void move(int x, int y);

private:
    HANDLE hSerial;
};

class Features
{
public:
    // Add shared position data
    static std::vector<Vector2> lastHeadScreenPositions;
    static std::vector<Vector3> lastHeadWorldPositions;
    static std::mutex positionDataMutex;

    Features();
    ~Features();

    void Kmbox();
    void UpdateAimbot();

    // Aimbot functionality
    Vector2 GetClosestEnemyInFOV();
    bool IsWithinFOV(const Vector2& screenPos, float fovRadius);
    void AimbotLogic();

private:
    KMBox kmbox;
    Vector2 lastAimTarget;
    bool aimbotActive;
};