#ifndef SOFTWARERENDERER_MATH_H
#define SOFTWARERENDERER_MATH_H

#include <cmath>

struct Vec3
{
    float x, y, z;

    Vec3 (float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    static Vec3 sum(const Vec3& a, const Vec3& b)
    {
        Vec3 c(a.x + b.x, a.y + b.y, a.z + b.z);
        return c;
    }

    static float dot(const Vec3& a, const Vec3& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static Vec3 cross(const Vec3& a, const Vec3& b)
    {
        Vec3 c(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
        return c;
    }

    void add(const Vec3& a)
    {
        x += a.x;
        y += a.y;
        z += a.z;
    }

    void sub(const Vec3& a)
    {
        x -= a.x;
        y -= a.y;
        z -= a.z;
    }

    void scale(float s)
    {
        x *= s;
        y *= s;
        z *= s;
    }

    void invert()
    {
        scale(-1);
    }

    float length()
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    void normalize()
    {
        float l = length();
        scale(1.0f / l);
    }
};

struct Vec4
{
    float x, y, z, w;

    Vec4()
    {
        this->x = 0;
        this->y = 0;
        this->z = 0;
        this->w = 1;
    }

    Vec4(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        w = 1;
    }

    Vec4(float x, float y, float z, float w)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }

    void scale(float s)
    {
        x *= s;
        y *= s;
        z *= s;
    }

    static Vec4 invert(const Vec4& v)
    {
        Vec4 inv_v = v;
        inv_v.scale(-1);
        return inv_v;
    }
};

struct Mat4
{
    float m[16];

    Mat4()
    {
        for (int i = 0; i < 16; i++)
            m[i] = 0;
    }

    static Mat4 identity()
    {
        Mat4 e;

        for (int i = 0; i < 4; i++)
            e.m[i*4 + i] = 1;

        return e;
    }

    static Vec4 mat4_multiply_vec4 (const Mat4& mat, const Vec4& vec)
    {
        Vec4 result = Vec4();

        result.x =  mat.m[0] * vec.x + mat.m[1] * vec.y + mat.m[2] * vec.z + mat.m[3] * vec.w;
        result.y =  mat.m[4] * vec.x + mat.m[5] * vec.y + mat.m[6] * vec.z + mat.m[7] * vec.w;
        result.z =  mat.m[8] * vec.x + mat.m[9] * vec.y + mat.m[10] * vec.z + mat.m[11] * vec.w;
        result.w =  mat.m[12] * vec.x + mat.m[13] * vec.y + mat.m[14] * vec.z + mat.m[15] * vec.w;

        return result;
    }

    static Mat4 mat4_translation(float tx, float ty, float tz)
    {
        Mat4 mat = identity();

        mat.m[3] = tx;
        mat.m[7] = ty;
        mat.m[11] = tz;

        return mat;
    }
};

#endif //SOFTWARERENDERER_MATH_H