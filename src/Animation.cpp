#include "Animation.h"
#include <math.h>

Animation::Animation()
{
}

Animation::Animation(const std::string & name, const sf::Texture & t)
    : Animation(name, "none", t, 1, 0, false)
{

}

Animation::Animation(const std::string & name, const std::string& entityName, const sf::Texture & t, size_t frameCount, size_t speed, bool addEntityAnimation)
    : m_name        (name)
	, m_entityName	(entityName)
    , m_sprite      (t)
    , m_frameCount  (frameCount)
    , m_currentFrame(0)
    , m_speed       (speed)
	, m_addAnimation (addEntityAnimation)
{
    m_size = Vec2((float)t.getSize().x / frameCount, (float)t.getSize().y);
    m_sprite.setOrigin(m_size.x / 2.0f, m_size.y / 2.0f);
    m_sprite.setTextureRect(sf::IntRect(floor(m_currentFrame) * m_size.x, 0, m_size.x, m_size.y));
}

// updates the animation to show the next frame, depending on its speed
// animation loops when it reaches the end
void Animation::update()
{
    if (m_frameCount < 2) { return; }

    // add the speed variable to the current frame
    m_currentFrame++;
    
    // set the currect texture based on the frame of animation
    size_t frame = (m_currentFrame / m_speed) % m_frameCount;
    m_sprite.setTextureRect(sf::IntRect(frame * m_size.x, 0, m_size.x, m_size.y));
}

const Vec2 & Animation::getSize() const
{
    return m_size;
}

const std::string & Animation::getName() const
{
    return m_name;
}

const std::string& Animation::getEntityName() const
{
	return m_entityName;
}

const bool& Animation::isAddAnimation() const
{
	return m_addAnimation;
}

sf::Sprite & Animation::getSprite()
{
    return m_sprite;
}

bool Animation::hasEnded() const
{
    return m_currentFrame >= m_frameCount * m_speed;
}