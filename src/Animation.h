#pragma once

#include "Common.h"
#include <vector>

class Animation
{
    sf::Sprite  m_sprite;
    size_t      m_frameCount    = 1;    // total number of frames of animation
    size_t      m_currentFrame  = 0; // the current frame of animation being played
    size_t      m_speed         = 0; // the speed to play this animation
    Vec2        m_size          = { 1, 1 }; // size of the animation frame
    std::string m_name = "none";
	std::string m_entityName = "none"; //the name of the entity this animation belongs to
	bool m_addAnimation = false; //if this is the animation shown when adding an entity

public:

    Animation();
    Animation(const std::string & name, const sf::Texture & t);
    Animation(const std::string& name, const std::string& entityName, const sf::Texture& t, size_t frameCount, size_t speed, bool addEntityAnimation);
        
    void update();
    bool hasEnded() const;
    const std::string & getName() const;
	const std::string& getEntityName() const;
	const bool& isAddAnimation() const;
    const Vec2 & getSize() const;
    sf::Sprite & getSprite();
};
