//
// Created by William Zhao on 5/10/26.
//

#include "QbertBall.h"

#include "ParabolicCurve.h"

QbertBall::QbertBall(Renderer *renderer, QbertLevel *level)
    : Actor(renderer, nullptr), m_level(level)
{
    m_jumpCurve = new ParabolicCurve(0.0f, 100.0f, -800.0f);
    SetScale(2.0f);
}

QbertBall::~QbertBall()
{
    delete m_jumpCurve;
}
