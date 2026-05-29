//
// Created by William Zhao on 5/7/26.
//

#ifndef GAME_CURVE_H
#define GAME_CURVE_H
#include "EngineMath.h"


class Curve
{
public:
    virtual ~Curve() = default;

    float Sample(const float t)
    {
        return Evaluate(Math::Clamp(t, 0.0f, 1.0f));
    }

    float SampleVelocity(const float t)
    {
        return EvaluateDerivative(Math::Clamp(t, 0.0f, 1.0f), 1);
    }

protected:
    virtual float Evaluate(const float t) = 0;

    virtual float EvaluateDerivative(const float t, const int degree) = 0;
};


#endif //GAME_CURVE_H
