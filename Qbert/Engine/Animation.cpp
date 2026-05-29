//
// Created by William Zhao on 3/22/26.
//

#include "Animation.h"
#include "JsonUtil.h"
#include "Skeleton.h"
#include "Profiler.h"

uint32_t Animation::GetNumBones() const
{
    return m_numBones;
}

uint32_t Animation::GetNumFrames() const
{
    return m_numFrames;
}

float Animation::GetLength() const
{
    return m_length;
}

const BoneTransform &Animation::GetBoneTransform(uint32_t frameIndex, uint32_t boneIndex) const
{
    return m_boneTransforms[boneIndex][frameIndex];
}

Animation *Animation::StaticLoad(const char *fileName, AssetManager *pAssetManager)
{
    Animation *pAnimation = new Animation();
    if (!pAnimation->Load(fileName))
    {
        delete pAnimation;
        return new Animation();
    }
    return pAnimation;
}

bool Animation::Load(const char *fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open())
        return false;
    rapidjson::IStreamWrapper isw(file);
    rapidjson::Document doc;
    doc.ParseStream(isw);
    if (!doc.IsObject())
        return false;
    std::string str = doc["metadata"]["type"].GetString();
    int ver = doc["metadata"]["version"].GetInt();
    // Check the metadata
    if (!doc["metadata"].IsObject() || str != "itpanim" || ver != 2)
        return false;
    auto &sequence = doc["sequence"];
    if (!sequence.IsObject())
        return false;
    if (!GetBoolFromJSON(sequence, "loop", m_isLooping))
        return false;
    if (!GetUintFromJSON(sequence, "frames", m_numFrames))
        return false;
    if (!GetFloatFromJSON(sequence, "length", m_length))
        return false;
    if (!GetUintFromJSON(sequence, "bonecount", m_numBones))
        return false;

    m_boneTransforms.resize(m_numBones);

    auto &tracks = sequence["tracks"];
    if (!tracks.IsArray())
        return false;

    for (rapidjson::SizeType i = 0; i < tracks.Size(); i++)
    {
        auto &track = tracks[i];
        if (!track.IsObject())
            return false;

        uint32_t boneIndex = 0;
        if (!GetUintFromJSON(track, "bone", boneIndex))
            return false;
        if (boneIndex >= m_numBones)
            return false;

        auto &transforms = track["transforms"];
        if (!transforms.IsArray())
            return false;
        if (transforms.Size() != m_numFrames)
            return false;

        // Allocate this bone's keyframe track
        m_boneTransforms[boneIndex].resize(m_numFrames);

        for (rapidjson::SizeType j = 0; j < m_numFrames; j++)
        {
            auto &transform = transforms[j];
            if (!transform.IsObject())
                return false;
            BoneTransform boneTransform;
            if (!GetQuaternionFromJSON(transform, "rot", boneTransform.rotation))
                return false;
            if (!GetVectorFromJSON(transform, "trans", boneTransform.translation))
                return false;
            m_boneTransforms[boneIndex][j] = boneTransform;
        }
    }
    return true;
}

void Animation::GetGlobalPoseAtTime(std::vector<Matrix4> &outPoses, const Skeleton *inSkeleton, float inTime) const
{
    PROFILE_SCOPE(AnimationGetGlobalPose);
    const size_t numBones = inSkeleton->GetNumBones();
    outPoses.resize(numBones);

    if (m_numFrames == 0 || m_length == 0.0f)
    {
        for (size_t i = 0; i < numBones; i++)
        {
            outPoses[i] = Matrix4::Identity;
        }
        return;
    }

    float timePerFrame = m_length / static_cast<float>(m_numFrames);
    uint32_t frameIndex;
    uint32_t nextFrame;
    float t;

    if (m_isLooping)
    {
        frameIndex = static_cast<uint32_t>(inTime / timePerFrame) % m_numFrames;
        nextFrame = (frameIndex + 1) % m_numFrames;
        t = (inTime - frameIndex * timePerFrame) / timePerFrame;
    } else
    {
        if (inTime >= m_length)
        {
            frameIndex = m_numFrames - 1;
            nextFrame = m_numFrames - 1;
            t = 1.0f;
        } else
        {
            frameIndex = static_cast<uint32_t>(inTime / timePerFrame);
            nextFrame = (frameIndex + 1 < m_numFrames) ? frameIndex + 1 : frameIndex;
            t = (inTime - frameIndex * timePerFrame) / timePerFrame;
        }
    }

    t = Math::Clamp(t, 0.0f, 1.0f);

    for (size_t i = 0; i < numBones; i++)
    {
        const auto &keyframes = m_boneTransforms[i];
        Matrix4 boneMatrix = Matrix4::Identity;

        if (!keyframes.empty())
        {
            BoneTransform bone = BoneTransform::Interpolate(keyframes[frameIndex], keyframes[nextFrame], t);
            boneMatrix = bone.ToMatrix();
        }

        const int parentIndex = inSkeleton->GetBone(i).parentIndex;
        if (parentIndex >= 0)
        {
            boneMatrix = boneMatrix * outPoses[parentIndex];
        }

        outPoses[i] = boneMatrix;
    }
}

