 #include "Assets.h"
#include <cassert>

Assets::Assets()
{

}

void Assets::loadFromFile(const std::string & path)
{
    std::ifstream file(path);
    std::string str;
    while (file.good())
    {
        file >> str;

        if (str == "Texture")
        {
            std::string name, path;
            file >> name >> path;
            addTexture(name, path);
        }
        else if (str == "Animation")
        {
            std::string name, texture, entityName;
            size_t frames, speed;
			bool addEntityAnimation;
            file >> name >> texture >> frames >> speed >> entityName >> addEntityAnimation;
            addAnimation(name, texture, entityName, frames, speed, addEntityAnimation);
			if (addEntityAnimation)
			{
				m_entityNames.push_back(entityName);
			}
        }
        else if (str == "Font")
        {
            std::string name, path;
            file >> name >> path;
            addFont(name, path);
        }
        else
        {
            std::cerr << "Unknown Asset Type: " << str << std::endl;
        }
    }
}

void Assets::addTexture(const std::string & textureName, const std::string & path, bool smooth)
{
    m_textureMap[textureName] = sf::Texture();

    if (!m_textureMap[textureName].loadFromFile(path))
    {
        std::cerr << "Could not load texture file: " << path << std::endl;
        m_textureMap.erase(textureName);
    }
    else
    {
        m_textureMap[textureName].setSmooth(smooth);
        std::cout << "Loaded Texture: " << path << std::endl;
    }
}

const sf::Texture & Assets::getTexture(const std::string & textureName) const
{
    assert(m_textureMap.find(textureName) != m_textureMap.end());
    return m_textureMap.at(textureName);
}

void Assets::addAnimation(const std::string& animationName, const std::string& textureName, const std::string& entityName, size_t frameCount, size_t speed, bool addEntityAnimation)
{
    m_animationMap[animationName] = Animation(animationName, entityName, getTexture(textureName), frameCount, speed, addEntityAnimation);
}

const Animation & Assets::getAnimation(const std::string & animationName) const
{
    assert(m_animationMap.find(animationName) != m_animationMap.end());
    return m_animationMap.at(animationName);
}

const std::vector<Animation> Assets::getEntityAnimation(const std::string& entityname) const
{
	std::vector<Animation> animationVector;
	for (auto const& pair : m_animationMap)
	{
		if (pair.second.getEntityName() == entityname)
		{
			animationVector.push_back(pair.second);
		}
	}
	return animationVector;
}

void Assets::addFont(const std::string & fontName, const std::string & path)
{
    m_fontMap[fontName] = sf::Font();
    if (!m_fontMap[fontName].loadFromFile(path))
    {
        std::cerr << "Could not load font file: " << path << std::endl;
        m_fontMap.erase(fontName);
    }
    else
    {
        std::cout << "Loaded Font:    " << path << std::endl;
    }
}

const sf::Font & Assets::getFont(const std::string & fontName) const
{
    assert(m_fontMap.find(fontName) != m_fontMap.end());
    return m_fontMap.at(fontName);
}

const std::vector<std::string>& Assets::getEntityNames() const
{
	return m_entityNames;
}
