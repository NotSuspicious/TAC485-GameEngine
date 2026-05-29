//
// Created by William Zhao on 3/22/26.
//

#ifndef GAME_ANIMATION_H
#define GAME_ANIMATION_H

#include "BoneTransform.h"
#include "JobManager.h"

class Animation
{
public:
    uint32_t GetNumBones() const;

    uint32_t GetNumFrames() const;

    float GetLength() const;

    bool IsLooping() const { return m_isLooping; }

    const BoneTransform &GetBoneTransform(uint32_t frameIndex, uint32_t boneIndex) const;

    static Animation *StaticLoad(const char *fileName, class AssetManager *pAssetManager);

    void GetGlobalPoseAtTime(
        std::vector<Matrix4> &outPoses, const class Skeleton *inSkeleton, float inTime
    ) const;

private:
    bool Load(const char *fileName);

    bool m_isLooping;
    uint32_t m_numBones;
    uint32_t m_numFrames;
    float m_length;
    std::vector<std::vector<BoneTransform> > m_boneTransforms; // [frame][bone]
};


#endif //GAME_ANIMATION_H
