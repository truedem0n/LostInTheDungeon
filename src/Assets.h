#pragma once

#include "Common.h"
#include "Animation.h"

class Assets
{
    std::map<std::string, sf::Texture>      m_textureMap;
    std::map<std::string, Animation>        m_animationMap;
    std::map<std::string, sf::Font>         m_fontMap;
	std::vector<std::string>				m_entityNames;

    void addTexture(const std::string & textureName, const std::string & path, bool smooth = true);
    void addAnimation(const std::string & animationName, const std::string & textureName, const std::string& entityName, size_t frameCount, size_t speed, bool addEntityAnimation);
    void addFont(const std::string & fontName, const std::string & path);

public:

    Assets();

    void loadFromFile(const std::string & path);

    const sf::Texture & getTexture(const std::string & textureName) const;
    const Animation &   getAnimation(const std::string & animationName) const;
	const std::vector<Animation> getEntityAnimation(const std::string & entityname) const;
    const sf::Font &    getFont(const std::string & fontName) const;
	const std::vector<std::string> & getEntityNames() const;
};