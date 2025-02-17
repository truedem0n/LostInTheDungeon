#pragma once

#include <bitset>
#include <array>
#include "Animation.h"
#include "Assets.h"

class Component
{
public:
    bool has = false;
};

class CTransform : public Component
{
public:
    Vec2 pos        = { 0.0, 0.0 };
    Vec2 prevPos    = { 0.0, 0.0 };
    Vec2 scale      = { 1.0, 1.0 };
    Vec2 speed      = { 0.0, 0.0 };
    Vec2 facing     = { 1.0, 0.0 };
    float angle = 0;

    CTransform(const Vec2 & p = { 0, 0 })
        : pos(p), angle(0) {}
    CTransform(const Vec2 & p, const Vec2 & sp, const Vec2 & sc, float a)
        : pos(p), prevPos(p), speed(sp), scale(sc), angle(a) {}

};

class CLifeSpan : public Component
{
public:
    sf::Clock clock;
    int lifespan = 0;
    CLifeSpan() {}
    CLifeSpan(int l) : lifespan(l) {}
};

class CInput : public Component
{
public:
    bool up         = false;
    bool down       = false;
    bool left       = false;
    bool right      = false;
    bool shoot      = false;
    bool canShoot   = true;

    CInput() {}
};

class CBoundingBox : public Component
{
public:
    Vec2 size;
    Vec2 halfSize;
    bool blockMove = false;
    bool blockVision = false;
    CBoundingBox() {}
    CBoundingBox(const Vec2 & s, bool m, bool v)
        : size(s), blockMove(m), blockVision(v), halfSize(s.x / 2, s.y / 2) {}
};

class CAnimation : public Component
{
public:
    Animation animation;
    bool repeat;
    CAnimation() {}
    CAnimation(const Animation & animation, bool r)
        : animation(animation), repeat(r) {}
};

class CGravity : public Component
{
public:
    Vec2 gravity;
    CGravity() {}
    CGravity(Vec2 g) : gravity(g) {}
};

class CHealth : public Component
{
public:
	int health;
	CHealth() {}
	CHealth(int g) : health(g) {}
};

class CState : public Component
{
public:
    std::string state = "attack";
    size_t frames = 0;
    CState() {}
    CState(const std::string & s) : state(s) {}
};

class CFollowPlayer : public Component
{
public:
    Vec2 home = { 0, 0 };
    float speed = 0;
	bool smartFollow = false;
	bool activated = false;
    CFollowPlayer() {}
    CFollowPlayer(Vec2 p, float s, bool f)
        : home(p), speed(s), smartFollow(f) {}
    
};

class CPatrol : public Component
{
public:
    std::vector<Vec2> positions;
    size_t currentPosition = 0;
    float speed = 0;
    CPatrol() {}
    CPatrol(std::vector<Vec2> & pos, float s) : positions(pos), speed(s) {}
};

class CDrag : public Component
{
public:
	bool drag = false;
	CDrag() {}
};

class CInventory : public Component
{
public:
	std::vector<Animation> items;
	std::vector<int> counts;
	CInventory() {}
};
