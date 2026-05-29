//
// Created by William Zhao on 5/7/26.
//

#ifndef GAME_PARABOLICCURVE_H
#define GAME_PARABOLICCURVE_H
#include "Curve.h"


class ParabolicCurve : public Curve
{
public:
    ParabolicCurve(float start, float end, float g = -4.0f);

protected:
    float Evaluate(const float t) override;

    float EvaluateDerivative(const float t, const int degree) override;

private:
    float m_startPosition;
    float m_endPosition;
    float m_g;
};


#endif //GAME_PARABOLICCURVE_H
