//
// Created by William Zhao on 5/10/26.
//

#ifndef GAME_SNAKESPHERE_H
#define GAME_SNAKESPHERE_H

#include "QbertBall.h"

class SnakeSphere : public QbertBall
{
public:
    SnakeSphere(class Renderer *renderer, class QbertLevel *level);

    void OnLand() override;
};


#endif //GAME_SNAKESPHERE_H
