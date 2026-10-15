#pragma once
#include <numbers>
#include <cmath>

//improvized Matrix4x4 (Chat_GPT)
struct Matrix4x4 {
	float m[4][4];
};

typedef struct D3DXMATRIX {
	float m[4][4];
} D3DXMATRIX, * LPD3DXMATRIX;

struct ViewMatrix {
	float matrix[4][4];
};

struct Vector2 {
	float x, y;

	constexpr Vector2(
		const float x = 0.f,
		const float y = 0.f) noexcept :
		x(x), y(y) {
	}

	constexpr Vector2 operator-(const Vector2& other) const noexcept
	{
		return Vector2{ x - other.x, y - other.y };
	}

	constexpr Vector2 operator+(const Vector2& other) const noexcept
	{
		return Vector2{ x + other.x, y + other.y };
	}

	constexpr Vector2 operator/(const float factor) const noexcept
	{
		return Vector2{ x / factor, y / factor };
	}

	constexpr Vector2 operator*(const float factor) const noexcept
	{
		return Vector2{ x * factor, y * factor };
	}

	// Add equality check
	constexpr bool operator==(const Vector2& other) const noexcept
	{
		return (x == other.x && y == other.y);
	}

	// Add static Zero function
	static constexpr Vector2 Zero() noexcept
	{
		return Vector2(0.0f, 0.0f);
	}
};

struct Vector3
{
	float x, y, z;

	constexpr Vector3(
		const float x = 0.f,
		const float y = 0.f,
		const float z = 0.f) noexcept :
		x(x), y(y), z(z) {
	}

	// FIXED: Return Vector3 objects instead of const Vector3&
	constexpr Vector3 operator-(const Vector3& other) const noexcept
	{
		return Vector3{ x - other.x, y - other.y, z - other.z };
	}

	constexpr Vector3 operator+(const Vector3& other) const noexcept
	{
		return Vector3{ x + other.x, y + other.y, z + other.z };
	}

	constexpr Vector3 operator/(const float factor) const noexcept
	{
		return Vector3{ x / factor, y / factor, z / factor };
	}

	constexpr Vector3 operator*(const float factor) const noexcept
	{
		return Vector3{ x * factor, y * factor, z * factor };
	}

	// Add equality check
	constexpr bool operator==(const Vector3& other) const noexcept
	{
		return (x == other.x && y == other.y && z == other.z);
	}

	// Add static Zero function
	static constexpr Vector3 Zero() noexcept
	{
		return Vector3(0.0f, 0.0f, 0.0f);
	}
};

struct Vector4 {
	float x, y, z, w;

	// Default and parameterized constructor
	constexpr Vector4(
		const float x = 0.f,
		const float y = 0.f,
		const float z = 0.f,
		const float w = 0.f) noexcept :
		x(x), y(y), z(z), w(w) {
	}

	// Subtraction operator
	constexpr Vector4 operator-(const Vector4& other) const noexcept
	{
		return Vector4{ x - other.x, y - other.y, z - other.z, w - other.w };
	}

	// Addition operator
	constexpr Vector4 operator+(const Vector4& other) const noexcept
	{
		return Vector4{ x + other.x, y + other.y, z + other.z, w + other.w };
	}

	// Division operator
	constexpr Vector4 operator/(const float factor) const noexcept
	{
		return Vector4{ x / factor, y / factor, z / factor, w / factor };
	}

	// Multiplication operator
	constexpr Vector4 operator*(const float factor) const noexcept
	{
		return Vector4{ x * factor, y * factor, z * factor, w * factor };
	}

	// Add equality check
	constexpr bool operator==(const Vector4& other) const noexcept
	{
		return (x == other.x && y == other.y && z == other.z && w == other.w);
	}
};