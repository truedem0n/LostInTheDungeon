#include "Physics.h"
#include "Components.h"

Vec2 Physics::GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
	auto& aSize = a->getComponent<CBoundingBox>().halfSize;
	auto& bSize = b->getComponent<CBoundingBox>().halfSize;
	auto& aVec = a->getComponent<CTransform>().pos;
	auto& bVec = b->getComponent<CTransform>().pos;
	auto delta = Vec2(abs(aVec.x - bVec.x), abs(aVec.y - bVec.y));
	float ox = (aSize.x + bSize.x) - delta.x;
	float oy = (aSize.y + bSize.y) - delta.y;
	return Vec2(ox, oy);
}

Vec2 Physics::GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
	auto& aSize = a->getComponent<CBoundingBox>().halfSize;
	auto& bSize = b->getComponent<CBoundingBox>().halfSize;
	auto& aVec = a->getComponent<CTransform>().prevPos;
	auto& bVec = b->getComponent<CTransform>().prevPos;
	auto delta = Vec2(abs(aVec.x - bVec.x), abs(aVec.y - bVec.y));
	float ox = (aSize.x + bSize.x) - delta.x;
	float oy = (aSize.y + bSize.y) - delta.y;
	return Vec2(ox, oy);
}

Intersect Physics::LineIntersect(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& d)
{
	Vec2 r = (b - a);
	Vec2 s = (d - c);
	float rxs = r.crossProduct(s);
	Vec2 cma = c - a;
	float t = (cma.crossProduct(s)) / rxs;
	float u = (cma.crossProduct(r)) / rxs;
	if (t >= 0 && t <= 1 && u >= 0 && u <= 1)
		return { true, Vec2(a.x + t * r.x,a.y + t * r.y) };
	else
		return { false, Vec2(0,0) };
}

bool Physics::EntityIntersect(const Vec2& a, const Vec2& b, std::shared_ptr<Entity> e)
{
	Vec2 vertexA(e->getComponent<CTransform>().pos.x - e->getComponent<CBoundingBox>().halfSize.x, e->getComponent<CTransform>().pos.y - e->getComponent<CBoundingBox>().halfSize.y);
	Vec2 vertexB(e->getComponent<CTransform>().pos.x + e->getComponent<CBoundingBox>().halfSize.x, e->getComponent<CTransform>().pos.y - e->getComponent<CBoundingBox>().halfSize.y);
	Vec2 vertexC(e->getComponent<CTransform>().pos.x + e->getComponent<CBoundingBox>().halfSize.x, e->getComponent<CTransform>().pos.y + e->getComponent<CBoundingBox>().halfSize.y);
	Vec2 vertexD(e->getComponent<CTransform>().pos.x - e->getComponent<CBoundingBox>().halfSize.x, e->getComponent<CTransform>().pos.y + e->getComponent<CBoundingBox>().halfSize.y);
	auto ab = Physics::LineIntersect(a, b, vertexA, vertexB);
	auto ad = Physics::LineIntersect(a, b, vertexA, vertexD);
	auto bc = Physics::LineIntersect(a, b, vertexB, vertexC);
	auto cd = Physics::LineIntersect(a, b, vertexC, vertexD);
	if (ab.result || ad.result || bc.result || cd.result)
	{
		return true;
	}
	else
	{
		return false;
	}
}