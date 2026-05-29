//
// Created by William Zhao on 2/4/26.
//

#ifndef DEBUGCUBE_H
#define DEBUGCUBE_H

#include "Actor.h"


class TestCube : public Actor
{
public:
    TestCube(class Renderer *renderer, class Material *material);

    ~TestCube() override;

private:
    class Renderer *m_renderer{};
};


#endif //DEBUGCUBE_H
