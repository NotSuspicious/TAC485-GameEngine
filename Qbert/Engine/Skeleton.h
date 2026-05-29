//
// Created by William Zhao on 3/22/26.
//

#ifndef GAME_SKELETON_H
#define GAME_SKELETON_H


#include "BoneTransform.h"

class Skeleton
{
public:
    struct Bone
    {
        BoneTransform localBindPose;
        std::string name;
        int parentIndex;
    };

    size_t GetNumBones() const;

    const Skeleton::Bone &GetBone(size_t idx) const;

    const std::vector<Skeleton::Bone> &GetBones() const;

    static Skeleton *StaticLoad(const char *fileName, class AssetManager *pAssetManager);

    std::vector<Matrix4> &GetGlobalInvBindPose() { return m_inverseBindPoseMatrices; }

private:
    std::vector<Bone> m_bones;
    std::vector<Matrix4> m_inverseBindPoseMatrices;

    void ComputeGlobalInvBindPose();

    bool Load(const char *fileName);
};


#endif //GAME_SKELETON_H
