#pragma once

enum class ActivationType
{
    NONE =      -1,
    RELU =       0,
    SIGMOID =    1,
    LEAKYRELU =  2,
    STEP      =  3
};


inline ActivationType fromInt(int x)
{
    switch (x)
    {
    case -1: return ActivationType::NONE;
    case 0:  return ActivationType::RELU;
    case 1:  return ActivationType::SIGMOID;
    case 2:  return ActivationType::LEAKYRELU;
    case 3:  return ActivationType::STEP;
    default: return ActivationType::RELU; // fallback
    }
}

inline int toInt(ActivationType a)
{
    return static_cast<int>(a);
}