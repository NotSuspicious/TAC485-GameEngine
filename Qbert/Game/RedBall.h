//
// Created by William Zhao on 5/10/26.
//

#ifndef GAME_REDBALL_H
#define GAME_REDBALL_H

#include "QbertBall.h"

class RedBall : public QbertBall
{
public:
    RedBall(class Renderer *renderer, class QbertLevel *level);
};


#endif //GAME_REDBALL_H
