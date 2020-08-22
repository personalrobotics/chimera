#pragma once

#include <iostream>
#include <string>

#include <pybind11/pybind11.h>

////////////////////////////////////////////////////////////////////////////////
//
// Following examples are from
// https://pybind11.readthedocs.io/en/stable/advanced/functions.html
//
////////////////////////////////////////////////////////////////////////////////

namespace chimera_test
{

//----------------------------------------
// Overriding virtual functions in Python
//----------------------------------------

namespace test1
{

class Animal
{
public:
    virtual ~Animal()
    {
    }
    virtual std::string go(int n_times) = 0;
};

class Dog : public Animal
{
public:
    Dog() = default; // TODO: Remove
    std::string go(int n_times) override
    {
        std::string result;
        for (int i = 0; i < n_times; ++i)
            result += "woof! ";
        return result;
    }
};

inline std::string call_go(Animal *animal)
{
    return animal->go(3);
}

} // namespace test1

//---------------------------------------------
// Combining virtual functions and inheritance
//---------------------------------------------

namespace test2
{

class Animal
{
public:
    virtual std::string go(int n_times) = 0;
    virtual std::string name()
    {
        return "unknown";
    }
};

class Dog : public Animal
{
public:
    std::string go(int n_times) override
    {
        std::string result;
        for (int i = 0; i < n_times; ++i)
            result += bark() + " ";
        return result;
    }
    virtual std::string bark()
    {
        return "woof!";
    }
};

class Husky : public Dog
{
};

inline std::string call_bark(Dog *animal)
{
    return animal->bark();
}

} // namespace test2

namespace test3
{

class Vector2
{
public:
    Vector2(float x, float y) : x(x), y(y)
    {
    }

    Vector2 operator+(const Vector2 &v) const
    {
        return Vector2(x + v.x, y + v.y);
    }

    Vector2 operator-(const Vector2 &v) const
    {
        return Vector2(x - v.x, y - v.y);
    }

    Vector2 operator*(const Vector2 &v) const
    {
        return Vector2(x * v.x, y * v.y);
    }

    Vector2 operator/(const Vector2 &v) const
    {
        return Vector2(x / v.x, y / v.y);
    }

    Vector2 operator+(float value) const
    {
        return Vector2(x + value, y + value);
    }

    Vector2 operator-(float value) const
    {
        return Vector2(x - value, y - value);
    }

    Vector2 operator*(float value) const
    {
        return Vector2(x * value, y * value);
    }

    Vector2 operator/(float value) const
    {
        return Vector2(x / value, y / value);
    }

    Vector2 &operator+=(const Vector2 &v)
    {
        x += v.x;
        y += v.y;
        return *this;
    }

    Vector2 &operator-=(const Vector2 &v)
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    Vector2 &operator*=(const Vector2 &v)
    {
        x *= v.x;
        y *= v.y;
        return *this;
    }

    Vector2 &operator/=(const Vector2 &v)
    {
        x /= v.x;
        y /= v.y;
        return *this;
    }

    Vector2 &operator+=(float v)
    {
        x += v;
        y += v;
        return *this;
    }

    Vector2 &operator-=(float v)
    {
        x -= v;
        y -= v;
        return *this;
    }

    Vector2 &operator*=(float v)
    {
        x *= v;
        y *= v;
        return *this;
    }

    Vector2 &operator/=(float v)
    {
        x /= v;
        y /= v;
        return *this;
    }

    // TODO: Operator function is not supported
    friend Vector2 operator+(float f, const Vector2 &v)
    {
        return Vector2(f + v.x, f + v.y);
    }

    // TODO: Operator function is not supported
    friend Vector2 operator-(float f, const Vector2 &v)
    {
        return Vector2(f - v.x, f - v.y);
    }

    // TODO: Operator function is not supported
    friend Vector2 operator*(float f, const Vector2 &v)
    {
        return Vector2(f * v.x, f * v.y);
    }

    // TODO: Operator function is not supported
    friend Vector2 operator/(float f, const Vector2 &v)
    {
        return Vector2(f / v.x, f / v.y);
    }

    float get_x() const
    {
        return x;
    }

    float get_y() const
    {
        return y;
    }

    std::string toString() const
    {
        return "[" + std::to_string(x) + ", " + std::to_string(y) + "]";
    }

private:
    float x, y;
};

} // namespace test3

//---------------------------------------------
// TODO: Add more sections...
//---------------------------------------------

} // namespace chimera_test
