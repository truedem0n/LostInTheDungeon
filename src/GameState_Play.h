#pragma once

#include "Common.h"
#include "GameState.h"
#include <map>
#include <memory>
#include <deque>

#include "EntityManager.h"

struct PlayerConfig
{
	float X, Y, CX, CY, SPEED, MAXSPEED, JUMP, GRAVITY;
	bool NPChasPathFinding = false;
	std::string WEAPON;
};

class GameState_Play : public GameState
{

protected:

	EntityManager           m_entityManager;
	std::shared_ptr<Entity> m_player;
	std::shared_ptr<Entity> m_changeAnimation;
	std::string             m_levelPath;
	std::string             m_fileName = "";
	sf::Text				m_fileNameToDraw;
	sf::Shader				shader, shader1, shader2;
	PlayerConfig            m_playerConfig;
	std::vector<Animation>	m_menuAnimations;
	size_t					m_menuIndex = 0;
	size_t					m_tileSize = 128;
	bool                    m_drawTextures = true;
	bool                    m_drawCollision = false;
	bool                    m_follow = false;
	bool					m_definitelySmellyZelda = false;
	bool					m_snapToGrid = false;
	bool					m_typing = false;
	bool					m_hasMenu = false;
	bool					m_addEntity = false;
	sf::Clock               m_clock;
	int navmesh[12][10];
	void init(const std::string& levelPath);

	void loadLevel(const std::string& filename);
	void deleteEntity();

	void initializeAddMenu();
	void initializeChangeAnimationMenu();
	void addEntity();
	void changeAnimation();

    void update();
    void spawnPlayer();
    void spawnSword(std::shared_ptr<Entity> entity);
	void inializeNavMesh();
	Vec2 resolveNavigation(int xPos, int yPos, float speed);

	void sMovement();
	void sAI();
	void sLifespan();
	void sUserInput();
	void sAnimation();
	void sCollision();
	void sCamera();
	void sRender();
	void sDrag();
	void sDrawGrid();
	void sDrawFileName(int x);
	void setEntityDraggable(int x, int y);
	void saveLevel();
	void sEntityCollision(std::shared_ptr<Entity> entity, std::shared_ptr<Entity> tiles);

public:

	GameState_Play(GameEngine& game, const std::string& levelPath);

};