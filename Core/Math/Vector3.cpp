#include "Vector3.h"

namespace Math
{
    const Vector3 Vector3::VectorZero = Vector3(0.0f, 0.0f, 0.0f);
    const Vector3 Vector3::VectorOne = Vector3(1.0f, 1.0f, 1.0f);

    Vector3::Vector3(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    float Vector3::SqrMagnitude() const
    {
        float result = x * x + y * y + z * z;
        return result;;
    }

    Vector3 Vector3::operator+(const Math::Vector3 &vec) const
    {
        Vector3 result;
        result.x = x + vec.x;
        result.y = y + vec.y;
        result.z = z + vec.z;
        return result;
    }

    Vector3 Vector3::operator-(const Vector3 &vec) const
    {
        Vector3 result;
        result.x = x - vec.x;
        result.y = y - vec.y;
        result.z = z - vec.z;
        return result;
    }

    Vector3 Vector3::operator*(const Vector3 &vec) const
    {
        Vector3 result;
        result.x = x * vec.x;
        result.y = y * vec.y;
        result.z = z * vec.z;
        return result;
    }
    Vector3 Vector3::operator/(const Vector3 &vec) const
    {
        Vector3 result;
        result.x = x / vec.x;
        result.y = y / vec.y;
        result.z = z / vec.z;
        return result;
    }

}// Math