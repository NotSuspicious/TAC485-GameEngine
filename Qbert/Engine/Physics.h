//
// Created by William Zhao on 4/6/26.
//

#ifndef GAME_PHYSICS_H
#define GAME_PHYSICS_H

#include "EngineMath.h"
class AABB;
class LineSegment;

class Physics
{
public:
    static bool Intersect(const AABB &a, const AABB &b, AABB *pOverlap = nullptr);

    static bool Intersect(const LineSegment &segment, const AABB &box,
                          Vector3 *pHitPoint = nullptr);

    static bool UnitTest();

    std::vector<const class CollisionBox *> m_collisionBoxes;

    void AddObj(const class CollisionBox *box);

    void RemoveObj(const class CollisionBox *box);

    bool SegmentCast(const LineSegment &segment, Vector3 *pHitPoint = nullptr);
};

class AABB : public Physics
{
public:
    AABB();

    AABB(Vector3 min, Vector3 max);

    Vector3 m_minCorner;
    Vector3 m_maxCorner;
};

class LineSegment : public Physics
{
public:
    LineSegment(Vector3 start, Vector3 end);

    Vector3 m_startPoint;
    Vector3 m_endPoint;
};

struct TestAABB
{
    AABB a;
    AABB b;
    AABB overlap;
};

struct TestSegment
{
    AABB box;
    LineSegment segment;
    bool hit;
    Vector3 point;
};

#endif //GAME_PHYSICS_H
