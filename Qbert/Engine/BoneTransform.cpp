//
// Created by William Zhao on 3/22/26.
//

#include "BoneTransform.h"
#include "EngineMath.h"

Matrix4 BoneTransform::ToMatrix() const
{
    return Matrix4::CreateFromQuaternion(rotation) * Matrix4::CreateTranslation(translation);
}

BoneTransform BoneTransform::Interpolate(const BoneTransform &a, const BoneTransform &b, float f)
{
    BoneTransform bone;
    bone.rotation = Quaternion::Slerp(a.rotation, b.rotation, f);
    bone.translation = Vector3::Lerp(a.translation, b.translation, f);

    return bone;
}
