#ifndef VECTOR4F_H
#define VECTOR4F_H

#include <iostream>
#include <math.h>

#include "PhotonBox/math/Vector3f.h"

class Vector4f
{
public:

	static const Vector4f ZERO;
	static const Vector4f ONE;

	static const Vector4f UNIT_X;
	static const Vector4f UNIT_Y;
	static const Vector4f UNIT_Z;
	static const Vector4f UNIT_W;


	Vector4f() : _x(0.0f), _y(0.0f), _z(0.0f), _w(0.0f) {}
	Vector4f(float val) : _x(val), _y(val), _z(val), _w(val) {}
	Vector4f(float x, float y, float z, float w) : _x(x), _y(y), _z(z), _w(w) {}
	Vector4f(Vector3f vec, float w) : _x(vec.x()), _y(vec.y()), _z(vec.z()), _w(w) {}
	Vector4f(float x, Vector3f vec) : _x(x), _y(vec.x()), _z(vec.y()), _w(vec.z()) {}

	float& x() { return _x; }
	float& y() { return _y; }
	float& z() { return _z; }
	float& w() { return _w; }

	float getX() const { return _x; }
	float getY() const { return _y; }
	float getZ() const { return _z; }
	float getW() const { return _w; }

	void setX(float value) { _x = value; }
	void setY(float value) { _y = value; }
	void setZ(float value) { _z = value; }
	void setW(float value) { _w = value; }

	bool equals(const Vector4f& other)
	{
		return (_x == other._x && _y == other._y && _z == other._z);
	}

	float length()
	{
		return sqrt(_x * _x + _y * _y + _z * _z + _w * _w);
	}

	float lengthSqrd()
	{
		return _x * _x + _y * _y + _z * _z + _w * _w;
	}

	float distance(const Vector4f& other)
	{
		return (*this - other).length();
	}

	float distanceSqrd(const Vector4f& other)
	{
		return (*this - other).lengthSqrd();
	}

	inline float dot(const Vector4f& other) const
	{
		return _x * other._z + _y * other._y + _z * other._z + _w * other._w;
	}

	inline Vector3f xyz() const
	{
		return Vector3f(_x, _y, _z);
	}

	inline Vector4f normalize() const
	{
		Vector4f ret = *this;
		return ret / ret.length();
	}

	inline bool operator== (const Vector4f& other) const
	{
		return (_x == other._x && _y == other._y && _z == other._z && _w == other._w);
	}

	inline bool operator!= (const Vector4f& other) const
	{
		return (_x != other._x && _y != other._y && _z != other._z && _w != other._w);
	}

	inline Vector4f operator/ (const float& scalar) const
	{
		return Vector4f(_x / scalar, _y / scalar, _z / scalar, _w / scalar);
	}

	inline Vector4f operator* (const float& scalar) const
	{
		return Vector4f(_x * scalar, _y * scalar, _z * scalar, _w * scalar);
	}

	inline Vector4f operator- (const Vector4f& other) const
	{
		return Vector4f(_x - other._x, _y - other._y, _z - other._z, _w - other._w);
	}

	inline Vector4f operator+ (const Vector4f& other) const
	{
		return Vector4f(_x + other._x, _y + other._y, _z + other._z, _w + other._w);
	}

	float& operator [](int index)
	{
		if (index == 0) return _x;
		else if (index == 1) return _y;
		else if (index == 2) return _z;
		else if (index == 3) return _w;
		else
		{
			std::cerr << "index " << std::to_string(index) << " out of range\n";
			return _w;
		}
	}

	float operator [](int index) const
	{
		if (index == 0) return _x;
		else if (index == 1) return _y;
		else if (index == 2) return _z;
		else if (index == 3) return _w;
		else
		{
			std::cerr << "index " << std::to_string(index) << " out of range\n";
			return _w;
		}
	}

	static Vector4f lerp(Vector4f start, Vector4f target, float percent);

	friend std::ostream& operator<<(std::ostream& os, Vector4f m)
	{
		return os << "(" << m.x() << ", " << m.y() << ", " << m.z() << ", " << m.w() << ")";
	}

	// PHYSX
	physx::PxVec4 toPhysX()
	{
		return physx::PxVec4(_x, _y, _z, _w);
	}
private:
	float _x, _y, _z, _w;
};

#endif // VECTOR4F_H
