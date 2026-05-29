//
// Created by William Zhao on 3/22/26.
//

#ifndef GAME_BONETRANSFORM_H
#define GAME_BONETRANSFORM_H

#include "EngineMath.h"


class BoneTransform
{
public:
    Quaternion rotation = Quaternion::Identity;
    Vector3 translation = Vector3::Zero;

    static BoneTransform Interpolate(
        const BoneTransform &a, const BoneTransform &b, float f);

    [[nodiscard]] Matrix4 ToMatrix() const;
};


#endif //GAME_BONETRANSFORM_H
