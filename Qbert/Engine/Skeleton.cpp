//
// Created by William Zhao on 3/22/26.
//

#include "Skeleton.h"
#include "AssetManager.h"
#include "JsonUtil.h"

void Skeleton::ComputeGlobalInvBindPose()
{
    std::vector<Matrix4> globalBindPoses(m_bones.size());

    for (size_t i = 0; i < m_bones.size(); ++i)
    {
        Matrix4 localPose = m_bones[i].localBindPose.ToMatrix();
        if (m_bones[i].parentIndex >= 0)
        {
            globalBindPoses[i] = localPose * globalBindPoses[m_bones[i].parentIndex];
        } else
        {
            globalBindPoses[i] = localPose;
        }
    }

    m_inverseBindPoseMatrices.resize(m_bones.size());
    for (size_t i = 0; i < m_bones.size(); ++i)
    {
        m_inverseBindPoseMatrices[i] = globalBindPoses[i];
        m_inverseBindPoseMatrices[i].Invert();
    }
}

bool Skeleton::Load(const char *fileName)
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
    if (!doc["metadata"].IsObject() || str != "itpskel" || ver != 1)
        return false;
    const rapidjson::Value &bonecount = doc["bonecount"];
    if (!bonecount.IsUint())
        return false;
    size_t count = bonecount.GetUint();
    m_bones.reserve(count);
    const rapidjson::Value &bones = doc["bones"];
    if (!bones.IsArray())
        return false;
    if (bones.Size() != count)
        return false;
    Bone temp;
    for (rapidjson::SizeType i = 0; i < count; i++)
    {
        if (!bones[i].IsObject())
            return false;
        const rapidjson::Value &name = bones[i]["name"];
        temp.name = name.GetString();
        const rapidjson::Value &parent = bones[i]["parent"];
        temp.parentIndex = parent.GetInt();
        const rapidjson::Value &bindpose = bones[i]["bindpose"];
        if (!bindpose.IsObject())
            return false;
        if (false == GetQuaternionFromJSON(bindpose, "rot", temp.localBindPose.rotation))
            return false;
        if (false == GetVectorFromJSON(bindpose, "trans", temp.localBindPose.translation))
            return false;
        m_bones.push_back(temp);
    }
    // Now that we have the bones
    ComputeGlobalInvBindPose();
    return true;
}

size_t Skeleton::GetNumBones() const
{
    return m_bones.size();
}

const Skeleton::Bone &Skeleton::GetBone(size_t idx) const
{
    return m_bones[idx];
}

const std::vector<Skeleton::Bone> &Skeleton::GetBones() const
{
    return m_bones;
}

Skeleton *Skeleton::StaticLoad(const char *fileName, AssetManager *pAssetManager)
{
    Skeleton *pSkeleton = new Skeleton();
    if (!pSkeleton->Load(fileName))
    {
        delete pSkeleton;
        return new Skeleton();
    }
    return pSkeleton;
}
