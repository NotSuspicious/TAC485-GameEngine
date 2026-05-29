//
// Created by William Zhao on 4/6/26.
//

#include "Physics.h"
#include "EngineMath.h"
#include "Components/CollisionBox.h"
#include "Profiler.h"

AABB::AABB(Vector3 min, Vector3 max) : m_minCorner(min), m_maxCorner(max)
{
}

AABB::AABB()
{
}

bool Physics::Intersect(const AABB &a, const AABB &b, AABB *pOverlap)
{
    bool intersect = (a.m_maxCorner.x >= b.m_minCorner.x && b.m_maxCorner.x >= a.m_minCorner.x)
                     && (a.m_maxCorner.y >= b.m_minCorner.y && b.m_maxCorner.y >= a.m_minCorner.y)
                     && (a.m_maxCorner.z >= b.m_minCorner.z && b.m_maxCorner.z >= a.m_minCorner.z);
    if (intersect && pOverlap != nullptr)
    {
        Vector3 min, max;
        min.x = Math::Max(a.m_minCorner.x, b.m_minCorner.x);
        min.y = Math::Max(a.m_minCorner.y, b.m_minCorner.y);
        min.z = Math::Max(a.m_minCorner.z, b.m_minCorner.z);
        max.x = Math::Min(a.m_maxCorner.x, b.m_maxCorner.x);
        max.y = Math::Min(a.m_maxCorner.y, b.m_maxCorner.y);
        max.z = Math::Min(a.m_maxCorner.z, b.m_maxCorner.z);
        pOverlap->m_minCorner = min;
        pOverlap->m_maxCorner = max;
    }
    return intersect;
}

bool Physics::Intersect(const LineSegment &segment, const AABB &box, Vector3 *pHitPoint)
{
    Vector3 delta = segment.m_endPoint - segment.m_startPoint;
    float d[3] = {delta.x, delta.y, delta.z};
    float p[3] = {segment.m_startPoint.x, segment.m_startPoint.y, segment.m_startPoint.z};
    float min[3] = {box.m_minCorner.x, box.m_minCorner.y, box.m_minCorner.z};
    float max[3] = {box.m_maxCorner.x, box.m_maxCorner.y, box.m_maxCorner.z};
    float tmin = 0.0f;
    float tmax = 1.0f;

    // For all three slabs
    for (int i = 0; i < 3; i++)
    {
        if (Math::IsZero(fabs(d[i])))
        {
            // Segment is parallel to slab. No hit if origin not within slab.
            if (p[i] < min[i] || p[i] > max[i]) return false;
        } else
        {
            // Compute segment parameter t of near/far plane intersections for this slab.
            float ood = 1.0f / d[i];
            float t1 = (min[i] - p[i]) * ood;
            float t2 = (max[i] - p[i]) * ood;
            if (t1 > t2) std::swap(t1, t2);

            if (t1 > tmin) tmin = t1;
            if (t2 < tmax) tmax = t2;
            if (tmin > tmax) return false;
        }
    }

    if (pHitPoint != nullptr)
    {
        Vector3 ret = segment.m_startPoint + delta * tmin;
        pHitPoint->Set(ret.x, ret.y, ret.z);
    }
    return true;
}

bool Physics::UnitTest()
{
    const TestAABB testAABB[13] =
    {
        {
            AABB(Vector3(0.0f, 0.0f, 0.0f), Vector3(100.0f, 100.0f, 100.0f)),
            AABB(Vector3(0.0f, 0.0f, 0.0f), Vector3(10.0f, 10.0f, 10.0f)),
            AABB(Vector3(0.0f, 0.0f, 0.0f), Vector3(10.0f, 10.0f, 10.0f))
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            AABB(Vector3(-110.0f, -10.0f, -10.0f), Vector3(-90.0f, 10.0f, 10.0f)),
            AABB(Vector3(-100.0f, -10.0f, -10.0f), Vector3(-90.0f, 10.0f, 10.0f))
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            AABB(Vector3(90.0f, -10.0f, -10.0f), Vector3(110.0f, 10.0f, 10.0f)),
            AABB(Vector3(90.0f, -10.0f, -10.0f), Vector3(100.0f, 10.0f, 10.0f))
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            AABB(Vector3(-10.0f, -110.0f, -10.0f), Vector3(10.0f, -90.0f, 10.0f)),
            AABB(Vector3(-10.0f, -100.0f, -10.0f), Vector3(10.0f, -90.0f, 10.0f))
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            AABB(Vector3(-10.0f, 90.0f, -10.0f), Vector3(10.0f, 110.0f, 10.0f)),
            AABB(Vector3(-10.0f, 90.0f, -10.0f), Vector3(10.0f, 100.0f, 10.0f))
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            AABB(Vector3(-10.0f, -10.0f, -110.0f), Vector3(10.0f, 10.0f, -90.0f)),
            AABB(Vector3(-10.0f, -10.0f, -100.0f), Vector3(10.0f, 10.0f, -90.0f))
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            AABB(Vector3(-10.0f, -10.0f, 90.0f), Vector3(10.0f, 10.0f, 110.0f)),
            AABB(Vector3(-10.0f, -10.0f, 90.0f), Vector3(10.0f, 10.0f, 100.0f))
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            AABB(Vector3(-120.0f, -10.0f, -10.0f), Vector3(-110.0f, 10.0f, 10.0f)),
            AABB(Vector3::One, Vector3::Zero)
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            AABB(Vector3(110.0f, -10.0f, -10.0f), Vector3(120.0f, 10.0f, 10.0f)),
            AABB(Vector3::One, Vector3::Zero)
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            AABB(Vector3(-10.0f, -120.0f, -10.0f), Vector3(10.0f, -110.0f, 10.0f)),
            AABB(Vector3::One, Vector3::Zero)
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            AABB(Vector3(-10.0f, 110.0f, -10.0f), Vector3(10.0f, 120.0f, 10.0f)),
            AABB(Vector3::One, Vector3::Zero)
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            AABB(Vector3(-10.0f, -10.0f, -120.0f), Vector3(10.0f, 10.0f, -110.0f)),
            AABB(Vector3::One, Vector3::Zero)
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            AABB(Vector3(-10.0f, -10.0f, 110.0f), Vector3(10.0f, 10.0f, 120.0f)),
            AABB(Vector3::One, Vector3::Zero)
        }
    };

    const TestSegment testSegment[] =
    {
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            LineSegment(Vector3(-110.0f, 0.0f, 0.0f), Vector3(-90.0f, 0.0f, 0.0f)),
            true, Vector3(-100.0f, 0.0f, 0.0f)
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            LineSegment(Vector3(0.0f, -110.0f, 0.0f), Vector3(0.0f, -90.0f, 0.0f)),
            true, Vector3(0.0f, -100.0f, 0.0f)
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            LineSegment(Vector3(0.0f, 0.0f, -110.0f), Vector3(0.0f, 0.0f, -90.0f)),
            true, Vector3(0.0f, 0.0f, -100.0f)
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            LineSegment(Vector3(110.0f, 0.0f, 0.0f), Vector3(90.0f, 0.0f, 0.0f)),
            true, Vector3(100.0f, 0.0f, 0.0f)
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            LineSegment(Vector3(0.0f, 110.0f, 0.0f), Vector3(0.0f, 90.0f, 0.0f)),
            true, Vector3(0.0f, 100.0f, 0.0f)
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            LineSegment(Vector3(0.0f, 0.0f, 110.0f), Vector3(0.0f, 0.0f, 90.0f)),
            true, Vector3(0.0f, 0.0f, 100.0f)
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            LineSegment(Vector3(-120.0f, 0.0f, 0.0f), Vector3(-110.0f, 0.0f, 0.0f)),
            false, Vector3::Zero
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            LineSegment(Vector3(0.0f, -120.0f, 0.0f), Vector3(0.0f, -110.0f, 0.0f)),
            false, Vector3::Zero
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            LineSegment(Vector3(0.0f, 0.0f, -120.0f), Vector3(0.0f, 0.0f, -110.0f)),
            false, Vector3::Zero
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            LineSegment(Vector3(120.0f, 0.0f, 0.0f), Vector3(110.0f, 0.0f, 0.0f)),
            false, Vector3::Zero
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            LineSegment(Vector3(0.0f, 120.0f, 0.0f), Vector3(0.0f, 110.0f, 0.0f)),
            false, Vector3::Zero
        },
        {
            AABB(Vector3(-100.0f, -100.0f, -100.0f), Vector3(100.0f, 100.0f, 100.0f)),
            LineSegment(Vector3(0.0f, 0.0f, 120.0f), Vector3(0.0f, 0.0f, 110.0f)),
            false, Vector3::Zero
        },
    };

    for (int i = 0; i < 13; i++)
    {
        TestAABB test = testAABB[i];
        AABB overlap(Vector3::One, Vector3::Zero);
        Physics::Intersect(test.a, test.b, &overlap);
        if (Vector3::IsCloseEnuf(overlap.m_maxCorner, test.overlap.m_maxCorner) && Vector3::IsCloseEnuf(
                overlap.m_minCorner, test.overlap.m_minCorner))
        {
        } else
        {
            return false;
        }
    }

    for (int i = 0; i < 12; i++)
    {
        TestSegment test = testSegment[i];
        Vector3 hitPoint = Vector3::Zero;
        bool isHit;
        isHit = Intersect(test.segment, test.box, &hitPoint);
        if (isHit != test.hit || (isHit && !Vector3::IsCloseEnuf(hitPoint, test.point)))
        {
            return false;
        }
    }
    return true;
}

void Physics::AddObj(const CollisionBox *box)
{
    m_collisionBoxes.push_back(box);
}

void Physics::RemoveObj(const CollisionBox *box)
{
    auto iter = std::find(m_collisionBoxes.begin(), m_collisionBoxes.end(), box);
    if (iter != m_collisionBoxes.end())
        m_collisionBoxes.erase(iter);
}

bool Physics::SegmentCast(const LineSegment &segment, Vector3 *pHitPoint)
{
    PROFILE_SCOPE(PhysicsSegmentCast)
    float minDist = (segment.m_endPoint - segment.m_startPoint).LengthSq();
    Vector3 point;
    bool isHit = false;
    for (auto collision: m_collisionBoxes)
    {
        AABB box = collision->GetAABB();
        if (Physics::Intersect(segment, box, pHitPoint))
            isHit = true;
        float dist = (segment.m_startPoint - *pHitPoint).LengthSq();
        if (dist < minDist)
        {
            minDist = dist;
            point = *pHitPoint;
        }
    }
    *pHitPoint = point;
    return isHit;
}

LineSegment::LineSegment(Vector3 start, Vector3 end) : m_startPoint(start), m_endPoint(end)
{
}
