#pragma once

#include "Common.h"
#include "GameState.h"
#include <map>
#include <memory>
#include <deque>

#include "EntityManager.h"



class GameState_Over : public GameState
{

protected:
	sf::Music				m_music;
	sf::Clock               m_clock;
	std::string             m_levelPath;
	void init(const std::string& levelPath);
	void loadLevel(const std::string& filename);

public:
	GameState_Over(GameEngine& game, const std::string& levelPath);

};