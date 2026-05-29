//
// Created by William Zhao on 5/7/26.
//

#include "ParabolicCurve.h"

ParabolicCurve::ParabolicCurve(const float start, const float end, float g)
    : m_startPosition(start), m_endPosition(end), m_g(g)
{
}

float ParabolicCurve::Evaluate(const float t)
{
    // Parabolic trajectory: y = y0 + v0*t + 0.5*g*t^2
    // Where v0 is computed so that at t=1, we reach endPosition
    // v0 = endPosition - startPosition - 0.5*g
    float v0 = m_endPosition - m_startPosition - 0.5f * m_g;
    return m_startPosition + v0 * t + 0.5f * m_g * t * t;
}

float ParabolicCurve::EvaluateDerivative(const float t, const int degree)
{
    if (degree == 1)
    {
        // Derivative: dy/dt = v0 + g*t
        float v0 = m_endPosition - m_startPosition - 0.5f * m_g;
        return v0 + m_g * t;
    }
    return 0;
}
