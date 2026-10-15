#pragma once
#include "Vector/Vector.hpp"

class World
{
public:
	void UpdateCam();
    Vector2 W2s(const Vector3& worldPos) const;
	World();
	~World();

private:
    // Camera data
    Vector3 position;
    ViewMatrix viewMatrix;
    ViewMatrix projectionMatrix;
    // Screen dimensions
    int screenWidth;
    int screenHeight;
};
