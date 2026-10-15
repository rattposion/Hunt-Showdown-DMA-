#include "World.hpp"
#include "Memory/Memory.h"
#include "Offsets/Offsets.hpp"
#include <iostream>

void World::UpdateCam()
{
	auto Camhandle = mem.CreateScatterHandle();

    //std::cout << "camera base: " << std::hex << CameraBase << std::endl;

    mem.AddScatterReadRequest(Camhandle, CameraBase + Offsets::CameraPosOffset, &position, sizeof(Vector3));

    mem.AddScatterReadRequest(Camhandle, CameraBase + Offsets::ViewMatrixOffset, &viewMatrix, sizeof(ViewMatrix));

    mem.AddScatterReadRequest(Camhandle, CameraBase + Offsets::ProjectionMatrixOffset, &projectionMatrix, sizeof(ViewMatrix));

    mem.ExecuteReadScatter(Camhandle);

	mem.CloseScatterHandle(Camhandle);

}

Vector2  World::W2s(const Vector3& pos) const
{


    if (position == Vector3::Zero())
        return Vector2::Zero();

    // Precompute matrix operations
    Vector4 transformed = {
        pos.x * viewMatrix.matrix[0][0] + pos.y * viewMatrix.matrix[1][0] + pos.z * viewMatrix.matrix[2][0] + viewMatrix.matrix[3][0],
        pos.x * viewMatrix.matrix[0][1] + pos.y * viewMatrix.matrix[1][1] + pos.z * viewMatrix.matrix[2][1] + viewMatrix.matrix[3][1],
        pos.x * viewMatrix.matrix[0][2] + pos.y * viewMatrix.matrix[1][2] + pos.z * viewMatrix.matrix[2][2] + viewMatrix.matrix[3][2],
        pos.x * viewMatrix.matrix[0][3] + pos.y * viewMatrix.matrix[1][3] + pos.z * viewMatrix.matrix[2][3] + viewMatrix.matrix[3][3]
    };

    // Check visibility
    if (transformed.z >= 0.0f)
        return Vector2::Zero();

    // Projection
    Vector4 projected = {
        transformed.x * projectionMatrix.matrix[0][0] + transformed.y * projectionMatrix.matrix[1][0] + transformed.z * projectionMatrix.matrix[2][0] + transformed.w * projectionMatrix.matrix[3][0],
        transformed.x * projectionMatrix.matrix[0][1] + transformed.y * projectionMatrix.matrix[1][1] + transformed.z * projectionMatrix.matrix[2][1] + transformed.w * projectionMatrix.matrix[3][1],
        transformed.x * projectionMatrix.matrix[0][2] + transformed.y * projectionMatrix.matrix[1][2] + transformed.z * projectionMatrix.matrix[2][2] + transformed.w * projectionMatrix.matrix[3][2],
        transformed.x * projectionMatrix.matrix[0][3] + transformed.y * projectionMatrix.matrix[1][3] + transformed.z * projectionMatrix.matrix[2][3] + transformed.w * projectionMatrix.matrix[3][3]
    };

    if (projected.w == 0.0f)
        return Vector2::Zero();

    // Divide by w
    projected.x /= projected.w;
    projected.y /= projected.w;

    // Get screen coordinates
    int width = 1920;
	int height = 1080;

    return Vector2{
        (1.0f + projected.x) * width * 0.5f,
        (1.0f - projected.y) * height * 0.5f
    };
}



World::World()
{
}

World::~World()
{
}