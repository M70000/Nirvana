#pragma once

#include <sstream>

class Vector
{
public:
    Vector(void)
    {
        Invalidate();
    }
    Vector(float X, float Y, float Z)
    {
        x = X;
        y = Y;
        z = Z;
    }
    Vector(const float* clr)
    {
        x = clr[0];
        y = clr[1];
        z = clr[2];
    }

    void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f)
    {
        x = ix; y = iy; z = iz;
    }

    bool IsValid() const
    {
        return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
    }
    void Invalidate()
    {
        x = y = z = std::numeric_limits<float>::infinity();
    }

    float& operator[](int i)
    {
        return ((float*)this)[i];
    }
    float operator[](int i) const
    {
        return ((float*)this)[i];
    }

    bool IsZero(float tolerance = 0.01f) const
    {
        return (x > -tolerance && x < tolerance&&
            y > -tolerance && y < tolerance&&
            z > -tolerance && z < tolerance);
    }

    void Zero()
    {
        x = y = z = 0.0f;
    }

    bool operator==(const Vector& src) const
    {
        return (src.x == x) && (src.y == y) && (src.z == z);
    }
    bool operator!=(const Vector& src) const
    {
        return (src.x != x) || (src.y != y) || (src.z != z);
    }

    Vector& operator+=(const Vector& v)
    {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }
    Vector& operator-=(const Vector& v)
    {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }
    Vector& operator*=(float fl)
    {
        x *= fl;
        y *= fl;
        z *= fl;
        return *this;
    }
    Vector& operator*=(const Vector& v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }
    Vector& operator/=(const Vector& v)
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        return *this;
    }
    Vector& operator+=(float fl)
    {
        x += fl;
        y += fl;
        z += fl;
        return *this;
    }
    Vector& operator/=(float fl)
    {
        x /= fl;
        y /= fl;
        z /= fl;
        return *this;
    }
    Vector& operator-=(float fl)
    {
        x -= fl;
        y -= fl;
        z -= fl;
        return *this;
    }

    void NormalizeInPlace()
    {
        *this = Normalized();
    }
    Vector Normalized() const
    {
        Vector res = *this;
        float l = res.Length();
        if(l != 0.0f) {
            res /= l;
        } else {
            res.x = res.y = res.z = 0.0f;
        }
        return res;
    }

#define M_PI 3.14159265358979323846

    auto toAngle() const noexcept
    {
        constexpr auto radiansToDegrees = [](float radians) noexcept { return radians * 180 / static_cast<float>(M_PI); };
        return Vector{ radiansToDegrees(std::atan2(-z, std::hypot(x, y))),
                       radiansToDegrees(std::atan2(y, x)),
                       0.0f };
    }

    static auto fromAngle(const Vector& angle) noexcept
    {
        constexpr auto degreesToRadians = [](float degrees) noexcept { return degrees * static_cast<float>(M_PI) / 180; };
        return Vector{ std::cos(degreesToRadians(angle.x)) * std::cos(degreesToRadians(angle.y)),
                       std::cos(degreesToRadians(angle.x)) * std::sin(degreesToRadians(angle.y)),
                      -std::sin(degreesToRadians(angle.x)) };
    }

    float DistTo(const Vector &vOther) const
    {
        Vector delta;

        delta.x = x - vOther.x;
        delta.y = y - vOther.y;
        delta.z = z - vOther.z;

        return delta.Length();
    }
    float DistToSqr(const Vector &vOther) const
    {
        Vector delta;

        delta.x = x - vOther.x;
        delta.y = y - vOther.y;
        delta.z = z - vOther.z;

        return delta.LengthSqr();
    }
    float Dot(const Vector& vOther) const
    {
        return (x*vOther.x + y*vOther.y + z*vOther.z);
    }

    void VectorCrossProduct(const Vector& a, const Vector& b, Vector& result)
    {
        result.x = a.y * b.z - a.z * b.y;
        result.y = a.z * b.x - a.x * b.z;
        result.z = a.x * b.y - a.y * b.x;
    }

    inline Vector Clamp()
    {

        if (this->x < -89.0f)
            this->x = -89.0f;

        if (this->x > 89.0f)
            this->x = 89.0f;

        while (this->y < -180.0f)
            this->y += 360.0f;

        while (this->y > 180.0f)
            this->y -= 360.0f;

        this->z = 0.0f;

        return *this;
    }

    Vector Cross(const Vector& vOther)
    {
        Vector res;
        VectorCrossProduct(*this, vOther, res);
        return res;
    }

    float Length() const
    {
        return sqrt(x*x + y*y + z*z);
    }

    Vector Angle(Vector* up = 0)
    {
        auto deg = [=](float a) -> double
        {
            return a * 57.295779513082;
        };

        if (!x && !y)
            return Vector(0, 0, 0);

        double roll = 0;
        if (up)
        {
            auto left = (*up).Cross(*this);
            roll = deg(atan2f(left.z, (left.y * x) - (left.x * y)));
        }
        return Vector(deg(atan2f(-z, sqrtf(x * x + y * y))), deg(atan2f(y, x)), roll);
    }

    float LengthSqr(void) const
    {
        return (x*x + y*y + z*z);
    }
    float Length2D() const
    {
        return sqrt(x*x + y*y);
    }

    Vector& operator=(const Vector &vOther)
    {
        x = vOther.x; y = vOther.y; z = vOther.z;
        return *this;
    }

    Vector Vector::operator-(void) const
    {
        return Vector(-x, -y, -z);
    }
    Vector Vector::operator+(const Vector& v) const
    {
        return Vector(x + v.x, y + v.y, z + v.z);
    }
    Vector Vector::operator-(const Vector& v) const
    {
        return Vector(x - v.x, y - v.y, z - v.z);
    }
    Vector Vector::operator*(float fl) const
    {
        return Vector(x * fl, y * fl, z * fl);
    }
    Vector Vector::operator*(const Vector& v) const
    {
        return Vector(x * v.x, y * v.y, z * v.z);
    }
    Vector Vector::operator/(float fl) const
    {
        return Vector(x / fl, y / fl, z / fl);
    }
    Vector Vector::operator/(const Vector& v) const
    {
        return Vector(x / v.x, y / v.y, z / v.z);
    }

    float x, y, z;
};

inline Vector operator*(float lhs, const Vector& rhs)
{
    return rhs * lhs;
}
inline Vector operator/(float lhs, const Vector& rhs)
{
    return rhs / lhs;
}

class __declspec(align(16)) VectorAligned : public Vector
{
public:
    inline VectorAligned(void) {};
    inline VectorAligned(float X, float Y, float Z)
    {
        Init(X, Y, Z);
    }

public:
    explicit VectorAligned(const Vector &vOther)
    {
        Init(vOther.x, vOther.y, vOther.z);
    }

    VectorAligned& operator=(const Vector &vOther)
    {
        Init(vOther.x, vOther.y, vOther.z);
        return *this;
    }

    VectorAligned& operator=(const VectorAligned &vOther)
    {
        Init(vOther.x, vOther.y, vOther.z);
        return *this;
    }

    float w;
};

class quaternion {
public:
    quaternion() : x(0), y(0), z(0), w(0) { }
    quaternion(float ix, float iy, float iz, float iw) : x(ix), y(iy), z(iz), w(iw) { }

    void init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f, float iw = 0.0f) { x = ix; y = iy; z = iz; w = iw; }

    float* base() { return reinterpret_cast<float*>(this); }
    const float* base() const { return reinterpret_cast<const float*>(this); }

    float x, y, z, w;
};