#pragma once

#include "Common.h"
#include "GameState.h"
#include <map>
#include <memory>
#include <deque>

#include "EntityManager.h"

class GameState_Menu : public GameState
{

protected:

    EntityManager               m_entityManager;
    std::string                 m_title;
    std::vector<std::string>    m_menuStrings;
    std::vector<std::string>    m_levelPaths;
    sf::Text                    m_menuText;
    size_t                      m_selectedMenuIndex = 0;
	bool						m_inMainMenu = true;
	sf::Music					m_music;
	sf::SoundBuffer				m_buffer;
	std::string                 m_charGenerator = "abcdefghijklmnopqrstuvwxyz";
	sf::Sound					m_sound;
    
    void init(const std::string & menuConfig);
    void update();
    void sUserInput();
    void sRender();

public:

    GameState_Menu(GameEngine & game);
	void GameState_Menu::playMusic();

};