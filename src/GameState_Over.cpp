#include "GameState_Over.h"
#include "GameState_Over.h"
#include "Common.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include <math.h>

GameState_Over::GameState_Over(GameEngine& game, const std::string& levelPath)
	: GameState(game)
	, m_levelPath(levelPath)
{
	init(m_levelPath);
}

void GameState_Over::init(const std::string& levelPath)
{
	if (!m_music.openFromFile("sounds/" + levelPath + ".wav"))
		std::cout << "error loading music file"; // error
	m_music.play();
	m_music.setLoop(true);
	m_music.setVolume(10);
}

