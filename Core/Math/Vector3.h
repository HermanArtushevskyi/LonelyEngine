#ifndef VECTOR3_H
#define VECTOR3_H

namespace Math
{
    struct Vector3
    {
    public:
        static const Vector3 VectorOne;
        static const Vector3 VectorZero;

        float x;
        float y;
        float z;

        explicit Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f);

        // Returns square of vector's magnitude. Use it if you do not need to know actual magnitude.
        float SqrMagnitude() const;

        Vector3 operator+(const Vector3 &vec) const;

        Vector3 operator-(const Vector3 &vec) const;

        Vector3 operator*(const Vector3 &vec) const;

        Vector3 operator/(const Vector3 &vec) const;
    };
}
#endif //VECTOR3_H
