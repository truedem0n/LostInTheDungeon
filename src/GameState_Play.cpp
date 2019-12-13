#include "GameState_Play.h"
#include "Common.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include <math.h>

GameState_Play::GameState_Play(GameEngine& game, const std::string& levelPath)
	: GameState(game)
	, m_levelPath(levelPath)
{
	init(m_levelPath);
}

void GameState_Play::init(const std::string& levelPath)
{
	loadLevel(levelPath);
}

void GameState_Play::loadLevel(const std::string& filename)
{
	//  source  in shader   file
	if (!shader.loadFromFile("shaderWater.frag", sf::Shader::Fragment))
	{
		// error...
	}

	//  source  in shader   file
	if (!shader1.loadFromFile("shaderRipple.frag", sf::Shader::Fragment))
	{
		// error...
	}

	if (!shader2.loadFromFile("shaderM.frag", sf::Shader::Fragment))
	{
		// error
		std::cout << "could not load shaderM.frag";
	}



	m_entityManager = EntityManager();

	std::ifstream infile(filename);
	//reading information from file
	std::string m_token;
	if (infile.is_open())
		while (infile.good())
		{
			infile >> m_token;
			if (m_token == "Player")
			{
				infile >> m_playerConfig.X >> m_playerConfig.Y >> m_playerConfig.CX
					>> m_playerConfig.CY >> m_playerConfig.SPEED;

			}
			else if (m_token == "Tile")
			{
				//Tile Specification :
				//Tile Name RX RY TX TY BM BV
				//	Animation Name    Name      string
				//	Room Coordinate   RX RY     int, int
				//	Tile Position     TX TY     int, int
				//	Blocks Movement   BM        int(1 = true, 0 = false)
				//	Blocks Vision     BV        int(1 = true, 0 = false)
				int  x = m_tileSize, y = m_tileSize, RX, RY, TX, TY;
				bool BM, BV;
				auto block = m_entityManager.addEntity("tile");
				infile >> m_token;
				// animation
				block->addComponent<CAnimation>(m_game.getAssets().
					getAnimation(m_token), true);
				// room coordinates, tile coordinates, block movement, and vision.
				infile >> RX >> RY >> TX >> TY >> BM >> BV;
				block->addComponent<CBoundingBox>(Vec2(x, y), BM, BV);
				block->addComponent<CTransform>(Vec2(x * TX + RX * float(m_game.window().getSize().x) + x / 2, y * TY + RY * float(m_game.window().getSize().y) + y / 2));
				block->getComponent<CTransform>().prevPos = block->getComponent<CTransform>().pos;
				block->addComponent<CDrag>();

				//remove this tile from the navigation mesh
				int navx = ((RX) * 10) + TX;
				int navy = ((RY + 1) * 6) + TY;
				if (BM)
				{
					navmesh[navy][navx] = -1;
				}
			}
			else if (m_token == "NPC")
			{
				//NPC Specification :
				//NPC Name RX RY TX TY BM BV AI ...
				//	Animation Name    Name      string
				//	Room Coordinate   RX RY     int, int
				//	Tile Position     TX TY     int, int
				//	Blocks Movement   BM        int(1 = true, 0 = false)
				//	Blocks Vision     BV        int(1 = true, 0 = false)
				//	AI Behavior Name  AI        string
				//	AI Parameters     ...       (see below)

				//	AI = Follow
				//	... = S
				//	Follow Speed      S         float(speed to follow player)

				//	AI = Patrol
				//	... = S N X1 Y1 X2 Y2 ... XN YN
				//	Patrol Speed      S         float
				//	Patrol Positions  N         int(number of patrol positions)
				//	Position 1 - N      Xi Yi     int, int(Tile Position of Patrol Position i)

				//	For Example :
				//NPC  Tektite  0  0 15 10 0 0 Patrol 2 4 15 10 15 7 17 7 17 10
				//	- Spawn an NPC with animation name Tektie in room(0, 0) with tile pos(15, 10)
				//	- This NPC does not block movement or vision
				//	- The NPC has a Patrol AI with speed 2 and 4 positions, each in room(0, 0)
				//	Positions : (15, 10) (15, 7) (17, 7) (17, 10)
				int x = m_tileSize, y = m_tileSize, RX, RY, TX, TY;
				bool BM, BV;
				std::string followBehaviour;
				auto npc = m_entityManager.addEntity("npc");
				// animation name
				infile >> m_token;
				npc->addComponent<CAnimation>(m_game.getAssets().
					getAnimation(m_token), true);
				// room coordinates, tile coordinates, block movement,vision, and behaviour.
				infile >> RX >> RY >> TX >> TY >> BM >> BV >> followBehaviour;
				npc->addComponent<CBoundingBox>(Vec2(96, 96), BM, BV);
				npc->addComponent<CTransform>(Vec2(x * TX + RX * float(m_game.window().getSize().x) + x / 2, y * TY + RY * float(m_game.window().getSize().y) + y / 2));
				npc->getComponent<CTransform>().prevPos = npc->getComponent<CTransform>().pos;
				if (followBehaviour == "Follow")
				{
					int speed;
					bool smartFollow;
					infile >> speed >> smartFollow;
					if (smartFollow == true)
					{
						m_playerConfig.NPChasPathFinding = true;
					}

					npc->addComponent<CFollowPlayer>(npc->getComponent<CTransform>().pos, speed, smartFollow);
				}
				else if (followBehaviour == "Patrol")
				{
					int speed, patrolPositions;
					infile >> speed >> patrolPositions;
					std::vector<Vec2> positions;
					for (int i = 0;i < patrolPositions;i++)
					{
						int Xi, Yi;
						infile >> Xi >> Yi;
						positions.push_back(Vec2(x * Xi + RX * float(m_game.window().getSize().x) + x / 2, y * Yi + RY * float(m_game.window().getSize().y) + y / 2));
					}
					npc->addComponent<CPatrol>(positions, speed);
					float angle = atan2(positions[0].y - npc->getComponent<CTransform>().pos.y, positions[0].x - npc->getComponent<CTransform>().pos.x);
					npc->getComponent<CTransform>().speed.x = cos(angle) * speed;
					npc->getComponent<CTransform>().speed.y = sin(angle) * speed;
				}
				npc->addComponent<CDrag>();
			}
		}
	infile.close();

	// spawn a player 
	spawnPlayer();
}

void GameState_Play::setEntityDraggable(int x, int y)
{
	for (auto& t : m_entityManager.getEntities())
	{
		sf::View m_roomView = m_game.window().getView();
		auto& tPos = t->getComponent<CTransform>().pos;
		auto& tAnimSize = t->getComponent<CAnimation>().animation.getSize();
		if (tPos.x + tAnimSize.x / 2 > x + m_roomView.getCenter().x - m_game.window().getSize().x / 2 && tPos.x - tAnimSize.x / 2 < x + m_roomView.getCenter().x - m_game.window().getSize().x / 2)
		{
			if (tPos.y + tAnimSize.y / 2 > y + m_roomView.getCenter().y - m_game.window().getSize().y / 2 && tPos.y - tAnimSize.y / 2 < y + m_roomView.getCenter().y - m_game.window().getSize().y / 2 && t->hasComponent<CDrag>())
			{
				t->getComponent<CDrag>().drag = !t->getComponent<CDrag>().drag;
			}
		}
	}
}

void GameState_Play::sDrag()
{
	// Only  works  for room view
	sf::View m_roomView = m_game.window().getView();
	for (auto t : m_entityManager.getEntities())
	{
		if (t->hasComponent<CDrag>())
		{
			if (t->getComponent<CDrag>().drag)
			{
				if (m_snapToGrid)
				{
					t->getComponent<CTransform>().pos.x = m_roomView.getCenter().x - m_game.window().getSize().x / 2 + (m_tileSize / 2) + (int(sf::Mouse::getPosition(m_game.window()).x) / m_tileSize) * m_tileSize;
					t->getComponent<CTransform>().pos.y = m_roomView.getCenter().y - m_game.window().getSize().y / 2 + (m_tileSize / 2) + (int(sf::Mouse::getPosition(m_game.window()).y) / m_tileSize) * m_tileSize;
				}
				else
				{
					t->getComponent<CTransform>().pos.x = m_roomView.getCenter().x - m_game.window().getSize().x / 2 + sf::Mouse::getPosition(m_game.window()).x;
					t->getComponent<CTransform>().pos.y = m_roomView.getCenter().y - m_game.window().getSize().y / 2 + sf::Mouse::getPosition(m_game.window()).y;
				}
			}
		}

	}
}


void GameState_Play::sDrawFileName(int x)
{
	if (x == 8)
		m_fileName = m_fileName.substr(0, m_fileName.size() - 1);
	else  if (x == 13)
	{
		if (m_fileName.length() > 0)
			std::cout << "File saved.";
		else
			std::cout << "Wrong file name.";
	}
	else
		m_fileName += static_cast<char>(x);
	Vec2 pos = m_player->getComponent<CTransform>().pos;
	Vec2 bound = m_player->getComponent<CBoundingBox>().halfSize;
	m_fileNameToDraw.setString(m_fileName);
	m_fileNameToDraw.setFont(m_game.getAssets().getFont("Megaman"));
	m_fileNameToDraw.setCharacterSize(30);
	m_fileNameToDraw.setOrigin(m_fileNameToDraw.getLocalBounds().width / 2, m_fileNameToDraw.getLocalBounds().height / 2);
	m_fileNameToDraw.setPosition(pos.x, pos.y);
}

void GameState_Play::sDrawGrid()
{
	sf::View m_roomView = m_game.window().getView();
	if (m_snapToGrid)
	{
		auto p = m_player->getComponent<CTransform>().pos;
		float RX = std::floor(p.x / m_game.window().getSize().x);
		float RY = std::floor(p.y / m_game.window().getSize().y);
		float height = std::floor(m_game.window().getSize().y);
		float width = std::floor(m_game.window().getSize().x);
		float roomX = width * RX;
		float roomY = height * RY;
		for (float x = roomX; x <= roomX + width; x += m_tileSize)
		{
			sf::Vertex line[] =
			{
				sf::Vertex(sf::Vector2f(x, roomY)),
				sf::Vertex(sf::Vector2f(x, roomY + height))

			};
			m_game.window().draw(line, 3, sf::Lines);
		}
		for (float y = roomY; y <= roomY + height; y += m_tileSize)
		{
			sf::Vertex line[] =
			{
				sf::Vertex(sf::Vector2f(roomX, y)),
				sf::Vertex(sf::Vector2f(roomX + width, y))

			};
			m_game.window().draw(line, 2, sf::Lines);
		}
	}
}

void GameState_Play::spawnPlayer()
{
	/*Player Specification :
	Player X Y BX BY S
		Spawn Position    X Y        int, int
		Bounding Box Size BX BY      int, int
		Speed             S          float*/
	m_player = m_entityManager.addEntity("player");
	m_player->addComponent<CTransform>(Vec2(m_playerConfig.X, m_playerConfig.Y));
	m_player->getComponent<CTransform>().prevPos = m_player->getComponent<CTransform>().pos;
	m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("StandDown"), true);
	// Cbounding box constructor needs bound size, blocksmovement, blocksvision
	m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CX, m_playerConfig.CY), true, true);
	m_player->addComponent<CInput>();

	// New element to CTransform: 'facing', to keep track of where the player is facing
	/*facing left = -1,0
	facing right = 1,0
	facing up = 0,-1
	facing down = 0,1
	*/
	m_player->getComponent<CTransform>().facing = Vec2(0, 1);
}

void GameState_Play::spawnSword(std::shared_ptr<Entity> entity)
{
	//Attacking:
	//	-When the player attacks, a sword appears for 150ms and then disappears
	//		approximately 1 tile away from the player in the direction they are facing.
	//		- The player's sword should be given a bounding box equal to the anim size
	//		- When the sword collides with an enemy, it destroys the enemy
	//		- When the player collides with an enemy, it respawns back at the start
	auto sword = m_entityManager.addEntity("sword");
	auto& entityFace = entity->getComponent<CTransform>().facing;
	sword->addComponent<CTransform>(entity->getComponent<CTransform>().pos + entityFace * 58);
	sword->getComponent<CTransform>().facing = entityFace;
	// if entity is facing left  or right
	if (entityFace.x != 0)
	{
		sword->addComponent<CAnimation>(m_game.getAssets().getAnimation("SwordRight"), true);
		sword->getComponent<CTransform>().scale.x = entityFace.x;
	}
	else
	{
		sword->addComponent<CAnimation>(m_game.getAssets().getAnimation("SwordUp"), true);
		sword->getComponent<CTransform>().scale.y = -entityFace.y;
	}
	sword->addComponent<CBoundingBox>(sword->getComponent<CAnimation>().animation.getSize(), false, false);
	sword->addComponent<CLifeSpan>(150);
}

void GameState_Play::update()
{

	m_entityManager.update();

	if (!m_paused)
	{
		shader.setUniform("time", m_clock.getElapsedTime().asSeconds());
		shader1.setUniform("time", m_clock.getElapsedTime().asSeconds());
		shader2.setUniform("time", m_clock.getElapsedTime().asSeconds());
		sAI();
		if (!m_typing)
			sMovement();
		sLifespan();
		sCollision();
		sAnimation();
		sDrag();
	}

	sCamera();
	sUserInput();
	sRender();
}

void GameState_Play::sMovement()
{
	//TODO: remove this before submission (helpful for debugging)
	//std::cout << m_player->getComponent<CTransform>().pos.x << "," << m_player->getComponent<CTransform>().pos.y << "\n";
	// Implement player velocity from input
	m_player->getComponent<CTransform>().speed.y = 0;
	m_player->getComponent<CTransform>().speed.x = 0;
	if (m_player->getComponent<CInput>().up)
	{
		m_player->getComponent<CTransform>().speed.y -= m_playerConfig.SPEED;
	}
	if (m_player->getComponent<CInput>().down)
	{
		m_player->getComponent<CTransform>().speed.y += m_playerConfig.SPEED;
	}
	if (m_player->getComponent<CInput>().right)
	{
		m_player->getComponent<CTransform>().speed.x += m_playerConfig.SPEED;
	}
	if (m_player->getComponent<CInput>().left)
	{
		m_player->getComponent<CTransform>().speed.x -= m_playerConfig.SPEED;
	}

	//if x and y speeds are given, move in the same direction as the previous frame
	if (m_player->getComponent<CTransform>().speed.y != 0 && m_player->getComponent<CTransform>().speed.x != 0)
	{
		if (m_player->getComponent<CTransform>().prevPos.x - m_player->getComponent<CTransform>().pos.x == 0)
		{
			m_player->getComponent<CTransform>().speed.x = 0;
		}
		else
		{
			m_player->getComponent<CTransform>().speed.y = 0;
		}
	}

	//Update the facing to reflect their movement
	if (m_player->getComponent<CTransform>().speed.y > 0)
	{
		m_player->getComponent<CTransform>().facing = Vec2(0, 1);
	}
	else if (m_player->getComponent<CTransform>().speed.y < 0)
	{
		m_player->getComponent<CTransform>().facing = Vec2(0, -1);
	}
	else if (m_player->getComponent<CTransform>().speed.x > 0)
	{
		m_player->getComponent<CTransform>().facing = Vec2(1, 0);
	}
	else if (m_player->getComponent<CTransform>().speed.x < 0)
	{
		m_player->getComponent<CTransform>().facing = Vec2(-1, 0);
	}

	//update character pos
	m_player->getComponent<CTransform>().prevPos = m_player->getComponent<CTransform>().pos;
	m_player->getComponent<CTransform>().pos += m_player->getComponent<CTransform>().speed;

	// check if the player moved tiles
	if (m_playerConfig.NPChasPathFinding && (floor(m_player->getComponent<CTransform>().pos.x / m_tileSize) - floor(m_player->getComponent<CTransform>().prevPos.x / m_tileSize) != 0 ||
		floor(m_player->getComponent<CTransform>().pos.y / m_tileSize) - floor(m_player->getComponent<CTransform>().prevPos.y / m_tileSize) != 0))
	{
		// initialize navigation mesh
		inializeNavMesh();
	}

	// Can spawnSword?
	if (m_player->getComponent<CInput>().shoot)
	{
		if (m_player->getComponent<CInput>().canShoot)
		{
			spawnSword(m_player);
			m_player->getComponent<CInput>().canShoot = false;
		}
	}

	auto& swords = m_entityManager.getEntities("sword");
	for (auto i : swords)
	{
		i->getComponent<CTransform>().pos = m_player->getComponent<CTransform>().pos + m_player->getComponent<CTransform>().facing * 58;
	}
	auto entity = m_entityManager.getEntities();
	for (auto t : entity)
	{

		if (t != m_player)
		{
			t->getComponent<CTransform>().prevPos = t->getComponent<CTransform>().pos;
			t->getComponent<CTransform>().pos += t->getComponent<CTransform>().speed;
		}
	}
}

void GameState_Play::inializeNavMesh()
{
	// make open and closed lists
	std::vector<Vec2> openList;
	std::vector<Vec2> initializedList;

	//initialize the player's position to 0
	int xPlayerTilePos = floor(m_player->getComponent<CTransform>().pos.x / m_tileSize);
	int yPlayerTilePos = floor(m_player->getComponent<CTransform>().pos.y / m_tileSize) + 6;
	navmesh[yPlayerTilePos][xPlayerTilePos] = 0;
	openList.push_back(Vec2(yPlayerTilePos, xPlayerTilePos));
	initializedList.push_back(Vec2(yPlayerTilePos, xPlayerTilePos));
	int count = 0;

	// reset navmesh
	for (int i = 0; i < 12; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (navmesh[i][j] != -1)
			{
				navmesh[i][j] = 0;
			}
		}
	}

	// BFS over the map
	while (!openList.empty())
	{
		Vec2 nextTile = openList[0];
		int i = 0;
		int j = -1;

		// check if tile is in bounds, can be moved through, and hasn't been updated
		if (nextTile.x + i < 12 && nextTile.x + i >= 0 && nextTile.y + j < 10 && nextTile.y + j >= 0 &&
			navmesh[int(nextTile.x) + i][int(nextTile.y) + j] != -1 && std::find(initializedList.begin(), initializedList.end(), Vec2(nextTile.x + i, nextTile.y + j)) == initializedList.end())
		{
			navmesh[int(nextTile.x) + i][int(nextTile.y) + j] = navmesh[int(nextTile.x)][int(nextTile.y)] + 1;
			openList.push_back(Vec2(nextTile.x + i, nextTile.y + j));
			initializedList.push_back(Vec2(nextTile.x + i, nextTile.y + j));
		}

		i = 0;
		j = 1;
		// check if tile is in bounds, can be moved through, and hasn't been updated
		if (nextTile.x + i < 12 && nextTile.x + i >= 0 && nextTile.y + j < 10 && nextTile.y + j >= 0 &&
			navmesh[int(nextTile.x) + i][int(nextTile.y) + j] != -1 && std::find(initializedList.begin(), initializedList.end(), Vec2(nextTile.x + i, nextTile.y + j)) == initializedList.end())
		{
			navmesh[int(nextTile.x) + i][int(nextTile.y) + j] = navmesh[int(nextTile.x)][int(nextTile.y)] + 1;
			openList.push_back(Vec2(nextTile.x + i, nextTile.y + j));
			initializedList.push_back(Vec2(nextTile.x + i, nextTile.y + j));
		}

		i = 1;
		j = 0;
		// check if tile is in bounds, can be moved through, and hasn't been updated
		if (nextTile.x + i < 12 && nextTile.x + i >= 0 && nextTile.y + j < 10 && nextTile.y + j >= 0 &&
			navmesh[int(nextTile.x) + i][int(nextTile.y) + j] != -1 && std::find(initializedList.begin(), initializedList.end(), Vec2(nextTile.x + i, nextTile.y + j)) == initializedList.end())
		{
			navmesh[int(nextTile.x) + i][int(nextTile.y) + j] = navmesh[int(nextTile.x)][int(nextTile.y)] + 1;
			openList.push_back(Vec2(nextTile.x + i, nextTile.y + j));
			initializedList.push_back(Vec2(nextTile.x + i, nextTile.y + j));
		}

		i = -1;
		j = 0;
		// check if tile is in bounds, can be moved through, and hasn't been updated
		if (nextTile.x + i < 12 && nextTile.x + i >= 0 && nextTile.y + j < 10 && nextTile.y + j >= 0 &&
			navmesh[int(nextTile.x) + i][int(nextTile.y) + j] != -1 && std::find(initializedList.begin(), initializedList.end(), Vec2(nextTile.x + i, nextTile.y + j)) == initializedList.end())
		{
			navmesh[int(nextTile.x) + i][int(nextTile.y) + j] = navmesh[int(nextTile.x)][int(nextTile.y)] + 1;
			openList.push_back(Vec2(nextTile.x + i, nextTile.y + j));
			initializedList.push_back(Vec2(nextTile.x + i, nextTile.y + j));
		}
		openList.erase(openList.begin());
	}
}

Vec2 GameState_Play::resolveNavigation(int xPos, int yPos, float speed)
{
	int lDist, rDist, uDist, dDist;

	if (xPos == 0 || navmesh[yPos - 1][xPos] == -1) { lDist = navmesh[yPos][xPos]; }
	else { lDist = navmesh[yPos - 1][xPos]; }

	if (xPos == 59 || navmesh[yPos + 1][xPos] == -1) { rDist = navmesh[yPos][xPos]; }
	else { rDist = navmesh[yPos + 1][xPos]; }

	if (yPos == 0 || navmesh[yPos][xPos - 1] == -1) { uDist = navmesh[yPos][xPos]; }
	else { uDist = navmesh[yPos][xPos - 1]; }

	if (yPos == 35 || navmesh[yPos][xPos + 1] == -1) { dDist = navmesh[yPos][xPos]; }
	else { dDist = navmesh[yPos][xPos + 1]; }

	float angle = atan2(uDist - dDist, lDist - rDist);
	return Vec2(sin(angle) * speed, cos(angle) * speed);
}

void GameState_Play::sAI()
{
	auto& entity = m_entityManager.getEntities();
	auto& npc = m_entityManager.getEntities("npc");
	for (auto t : npc)
	{
		// Patrol behaviour
		if (t->hasComponent<CPatrol>())
		{
			Vec2 currentPos = t->getComponent<CPatrol>().positions[t->getComponent<CPatrol>().currentPosition];
			if (abs(t->getComponent<CTransform>().pos.x - currentPos.x) < 5 && abs(t->getComponent<CTransform>().pos.y - currentPos.y) < 5)
			{
				t->getComponent<CPatrol>().currentPosition = (t->getComponent<CPatrol>().currentPosition + 1) % t->getComponent<CPatrol>().positions.size();
				currentPos = t->getComponent<CPatrol>().positions[t->getComponent<CPatrol>().currentPosition];
				float angle = atan2(currentPos.y - t->getComponent<CTransform>().pos.y, currentPos.x - t->getComponent<CTransform>().pos.x);
				t->getComponent<CTransform>().speed.x = cos(angle) * t->getComponent<CPatrol>().speed;
				t->getComponent<CTransform>().speed.y = sin(angle) * t->getComponent<CPatrol>().speed;

				//update the npc's facing
				if (t->getComponent<CTransform>().speed.x > 0.1)
				{
					t->getComponent<CTransform>().facing.x = 1;
				}
				else if (t->getComponent<CTransform>().speed.x < -0.1)
				{
					t->getComponent<CTransform>().facing.x = -1;
				}
				else
				{
					t->getComponent<CTransform>().facing.x = 0;
				}

				//update the npc's facing
				if (t->getComponent<CTransform>().speed.y > 0.1)
				{
					t->getComponent<CTransform>().facing.y = 1;
				}
				else if (t->getComponent<CTransform>().speed.y < -0.1)
				{
					t->getComponent<CTransform>().facing.y = -1;
				}
				else
				{
					t->getComponent<CTransform>().facing.y = 0;
				}
			}
		}

		// Follow behaviour
		if (t->hasComponent<CFollowPlayer>())
		{
			bool blocked = false;
			for (auto thisEntity : entity)
			{

				// if thisEntity's vision is blocked then set blocked true 
				if (thisEntity->hasComponent<CBoundingBox>())
				{
					if (thisEntity->getComponent<CBoundingBox>().blockVision == true)
					{
						if (thisEntity != t && thisEntity != m_player)
						{
							bool intersect = Physics::EntityIntersect(m_player->getComponent<CTransform>().pos, t->getComponent<CTransform>().pos, thisEntity);
							if (intersect)
							{
								blocked = true;
								break;

							}
						}

					}
				}

			}
			if (t->getComponent<CFollowPlayer>().smartFollow&& t->getComponent<CFollowPlayer>().activated)
			{
				//Get a vector for each tile the enemy overlaps so the enemy doesn't get stuck
				float xPos = (t->getComponent<CTransform>().pos.x / m_tileSize) - 0.5;
				float yPos = (t->getComponent<CTransform>().pos.y / m_tileSize) - 0.5;
				int xPos2 = floor(xPos);
				int yPos2 = floor(yPos) + 6;
				Vec2 speed1 = resolveNavigation(xPos2, yPos2, t->getComponent<CFollowPlayer>().speed);

				xPos2 = ceil(xPos);
				yPos2 = floor(yPos) + 6;
				Vec2 speed2 = resolveNavigation(xPos2, yPos2, t->getComponent<CFollowPlayer>().speed);

				xPos2 = floor(xPos);
				yPos2 = ceil(yPos) + 6;
				Vec2 speed3 = resolveNavigation(xPos2, yPos2, t->getComponent<CFollowPlayer>().speed);

				xPos2 = ceil(xPos);
				yPos2 = ceil(yPos) + 6;
				Vec2 speed4 = resolveNavigation(xPos2, yPos2, t->getComponent<CFollowPlayer>().speed);
				speed1 += speed2;
				speed1 += speed3;
				speed1 += speed4;
				t->getComponent<CTransform>().speed = speed1 / 4;
			}
			// if npc can see player 
			else if (!blocked)
			{
				if (t->getComponent<CFollowPlayer>().smartFollow)
				{
					t->getComponent<CFollowPlayer>().activated = true;
				}
				else
				{
					Vec2 delta = m_player->getComponent<CTransform>().pos - t->getComponent<CTransform>().pos;
					float length = delta.dist(Vec2(0, 0));
					delta = delta / length;

					// special zelda only move
					// press C to activate
					if (m_definitelySmellyZelda && t->hasComponent<CBoundingBox>())
					{
						t->getComponent<CTransform>().speed = Vec2(-1 * delta.x * t->getComponent<CFollowPlayer>().speed, -1 * delta.y * t->getComponent<CFollowPlayer>().speed);
					}
					// other wise follow the player
					else if (t->hasComponent<CBoundingBox>())
					{
						t->getComponent<CTransform>().speed = (delta * t->getComponent<CFollowPlayer>().speed);
					}
				}
			}
			// if player being followed by npc is lost then return to home
			else if (t->getComponent<CFollowPlayer>().home != t->getComponent<CTransform>().pos && t->hasComponent<CBoundingBox>())
			{
				Vec2 delta = t->getComponent<CFollowPlayer>().home - t->getComponent<CTransform>().pos;
				float length = delta.dist(Vec2(0, 0));
				delta = delta / length;
				t->getComponent<CTransform>().speed = (delta * t->getComponent<CFollowPlayer>().speed);
				if (abs(t->getComponent<CFollowPlayer>().home.x - t->getComponent<CTransform>().pos.x) < 1 && abs(t->getComponent<CFollowPlayer>().home.y - t->getComponent<CTransform>().pos.y) < 1)
				{
					t->getComponent<CTransform>().pos = t->getComponent<CFollowPlayer>().home;
					t->getComponent<CTransform>().speed = Vec2(0, 0);
				}
			}
		}
	}
}


void GameState_Play::sLifespan()
{
	// Implement Lifespan
	auto& entities = m_entityManager.getEntities();
	for (auto i : entities)
	{
		if (i->hasComponent<CLifeSpan>() && i->getComponent<CLifeSpan>().clock.getElapsedTime().asMilliseconds() > i->getComponent<CLifeSpan>().lifespan)
		{
			if (i->getComponent<CAnimation>().animation.getName() == "SwordRight" || i->getComponent<CAnimation>().animation.getName() == "SwordUp")
			{
				m_player->getComponent<CInput>().canShoot = true;
			}

			i->destroy();
		}
	}
}

void GameState_Play::sEntityCollision(std::shared_ptr<Entity> entity, std::shared_ptr<Entity> tile)
{
	Vec2 currOverlap = Physics::GetOverlap(entity, tile);
	Vec2 prevOverlap = Physics::GetPreviousOverlap(entity, tile);
	if (currOverlap.x > 0 || currOverlap.y > 0)
	{
		if (prevOverlap.y > 0)
		{
			if (currOverlap.x > 0)
			{
				//left side collision
				if (entity->getComponent<CTransform>().pos.x < tile->getComponent<CTransform>().pos.x)
				{
					entity->getComponent<CTransform>().pos.x -= currOverlap.x;
				}
				//right side collision
				else if (entity->getComponent<CTransform>().pos.x > tile->getComponent<CTransform>().pos.x)
				{
					entity->getComponent<CTransform>().pos.x += currOverlap.x;
				}
			}
		}
		if (prevOverlap.x > 0)
		{
			if (currOverlap.y > 0)
			{
				//Top collision
				if (entity->getComponent<CTransform>().pos.y < tile->getComponent<CTransform>().pos.y)
				{
					entity->getComponent<CTransform>().pos.y -= currOverlap.y;
				}
				//bottom collision
				else if (entity->getComponent<CTransform>().pos.y > tile->getComponent<CTransform>().pos.y)
				{
					entity->getComponent<CTransform>().pos.y += currOverlap.y;
				}
			}

		}
	}
}

void GameState_Play::saveLevel()
{
	m_fileName += ".txt";
	std::ofstream writeFile(m_fileName);
	//Tile Specification :
				//Tile Name RX RY TX TY BM BV
				//	Animation Name    Name      string
				//	Room Coordinate   RX RY     int, int
				//	Tile Position     TX TY     int, int
				//	Blocks Movement   BM        int(1 = true, 0 = false)
				//	Blocks Vision     BV        int(1 = true, 0 = false)
	int width = m_game.window().getSize().x;
	int height = m_game.window().getSize().y;
	for (auto tiles : m_entityManager.getEntities("tile"))
	{
		std::string animationName;
		int RX, RY, TX, TY, BM, BV;
		animationName = tiles->getComponent<CAnimation>().animation.getName() + " ";
		auto transform = tiles->getComponent<CTransform>();
		RX = std::floor(transform.pos.x / width);
		RY = std::floor(transform.pos.y / height);
		TX = (transform.pos.x - RX * float(width)) / m_tileSize;
		TY = (transform.pos.y - RY * float(height)) / m_tileSize;
		BM = tiles->getComponent<CBoundingBox>().blockMove;
		BV = tiles->getComponent<CBoundingBox>().blockVision;
		writeFile << "Tile " << animationName << RX << " " << RY << " " << TX << " " << TY << " " << BM << " " << BV << "\n";
	}
	//NPC Specification :
				//NPC Name RX RY TX TY BM BV AI ...
				//	Animation Name    Name      string
				//	Room Coordinate   RX RY     int, int
				//	Tile Position     TX TY     int, int
				//	Blocks Movement   BM        int(1 = true, 0 = false)
				//	Blocks Vision     BV        int(1 = true, 0 = false)
				//	AI Behavior Name  AI        string
				//	AI Parameters     ...       (see below)

				//	AI = Follow
				//	... = S
				//	Follow Speed      S         float(speed to follow player)
				//  PathFinding       P         int(1 = true, 0 = false)

				//	AI = Patrol
				//	... = S N X1 Y1 X2 Y2 ... XN YN
				//	Patrol Speed      S         float
				//	Patrol Positions  N         int(number of patrol positions)
				//	Position 1 - N      Xi Yi     int, int(Tile Position of Patrol Position i)

				//	For Example :
				//NPC  Tektite  0  0 15 10 0 0 Patrol 2 4 15 10 15 7 17 7 17 10
				//	- Spawn an NPC with animation name Tektie in room(0, 0) with tile pos(15, 10)
				//	- This NPC does not block movement or vision
				//	- The NPC has a Patrol AI with speed 2 and 4 positions, each in room(0, 0)
				//	Positions : (15, 10) (15, 7) (17, 7) (17, 10)
	for (auto npc : m_entityManager.getEntities("npc"))
	{
		std::string animationName;
		int RX, RY, TX, TY, BM, BV;
		animationName = npc->getComponent<CAnimation>().animation.getName() + " ";
		auto transform = npc->getComponent<CTransform>();
		RX = std::floor(transform.pos.x / width);
		RY = std::floor(transform.pos.y / height);
		TX = (transform.pos.x - RX * float(width)) / m_tileSize;
		TY = (transform.pos.y - RY * float(height)) / m_tileSize;
		BM = npc->getComponent<CBoundingBox>().blockMove;
		BV = npc->getComponent<CBoundingBox>().blockVision;
		if (npc->hasComponent<CFollowPlayer>())
		{
			std::string npcBehaviour = "Follow ";
			int speed, smartFollow;
			speed = npc->getComponent<CFollowPlayer>().speed;
			smartFollow = npc->getComponent<CFollowPlayer>().smartFollow;
			writeFile << "NPC " << animationName << RX << " " << RY << " " << TX << " " << TY << " " << BM << " " << BV << " " << npcBehaviour
				<< speed << " " << smartFollow << "\n";
		}
		else if (npc->hasComponent<CPatrol>())
		{
			std::string npcBehaviour = "Patrol ";
			int speed, nPositions;
			nPositions = npc->getComponent<CPatrol>().positions.size();
			speed = npc->getComponent<CPatrol>().speed;
			std::vector<Vec2> positions = npc->getComponent<CPatrol>().positions;

			writeFile << "NPC " << animationName << RX << " " << RY << " " << TX << " " << TY << " " << BM << " " << BV << " " << npcBehaviour
				<< speed << " " << nPositions << " ";
			for (Vec2 i : positions)
			{
				writeFile << int(i.x / m_tileSize) << " " << int(i.y / m_tileSize) << " ";
			}
			writeFile << "\n";
		}
	}


	//Player Specification :
	//Player X Y BX BY S
	//	Spawn Position    X Y        int, int
	//	Bounding Box Size BX BY      int, int
	//	Speed             S          float
	writeFile << "Player " << m_playerConfig.X << " " << m_playerConfig.Y << " " << m_playerConfig.CX << " " << m_playerConfig.CY << " " << m_playerConfig.SPEED << "\n";


	writeFile.close();

	// After writing is done
	m_fileName = "";
	m_fileNameToDraw.setString(m_fileName);
	m_typing = !m_typing;
}

void GameState_Play::sCollision()
{
	// Implement Collision detection / resolution
	auto& tile = m_entityManager.getEntities("tile");
	for (auto i : tile)
	{
		auto npc = m_entityManager.getEntities("npc");
		if (i->getComponent<CBoundingBox>().blockMove)
		{
			for (auto n : npc)
			{
				sEntityCollision(n, i);
			}
			sEntityCollision(m_player, i);
		}
	}

	// sword enemy player collision
	auto& enemies = m_entityManager.getEntities("npc");
	auto& sword = m_entityManager.getEntities("sword");
	for (auto i : enemies)
	{
		auto overlap = Physics::GetOverlap(i, m_player);
		if (overlap.x > 0 && overlap.y > 0)
		{
			if (i->hasComponent<CBoundingBox>()) {
				m_player->destroy();
				spawnPlayer();
			}
		}

		for (auto j : sword)
		{
			auto currOverlap = Physics::GetOverlap(i, j);
			auto prevOverlap = Physics::GetPreviousOverlap(i, j);

			if (currOverlap.x > 0 && currOverlap.y > 0)
			{
				if (i->hasComponent<CBoundingBox>()) {
					i->getComponent<CTransform>().speed = Vec2(0, 0);
					i->removeComponent<CBoundingBox>();
					i->getComponent<CAnimation>().animation =
						m_game.getAssets().getAnimation("Explosion");
					i->getComponent<CAnimation>().repeat = false;
					if (i->hasComponent<CFollowPlayer>())
					{
						i->getComponent<CFollowPlayer>().smartFollow = false;
					}
				}


			}
		}
	}
}


void GameState_Play::sUserInput()
{
	sf::Event event;
	while (m_game.window().pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			m_game.quit();
		}

		// this event is triggered when a key is pressed
		if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::Escape: { m_game.popState(); break; }
			case sf::Keyboard::Z: { init(m_levelPath); break; }
			case sf::Keyboard::R: { m_drawTextures = !m_drawTextures; break; }
			case sf::Keyboard::F: { m_drawCollision = !m_drawCollision; break; }
			case sf::Keyboard::Y: { m_follow = !m_follow; break; }
			case sf::Keyboard::P: { setPaused(!m_paused); break; }

			case sf::Keyboard::Insert: { if (!m_hasMenu) { initializeAddMenu(); } break; }
			case sf::Keyboard::M: { if (!m_hasMenu) { initializeChangeAnimationMenu(); } break; }
			case sf::Keyboard::Down: { if (m_menuIndex < m_menuAnimations.size() - 1) { m_menuIndex++; } break; }
			case sf::Keyboard::Up: { if (m_menuIndex > 0) { m_menuIndex--; } break; }

			case sf::Keyboard::W: { m_player->getComponent<CInput>().up = true; break; }
			case sf::Keyboard::S: { m_player->getComponent<CInput>().down = true; break; }
			case sf::Keyboard::A: { m_player->getComponent<CInput>().left = true; break; }
			case sf::Keyboard::C: { m_definitelySmellyZelda = !m_definitelySmellyZelda; break; }
			case sf::Keyboard::D: { m_player->getComponent<CInput>().right = true; break; }
			case sf::Keyboard::Space: { m_player->getComponent<CInput>().shoot = true; break; }
			case sf::Keyboard::G: {m_snapToGrid = !m_snapToGrid; break; }
			case sf::Keyboard::F1: {m_typing = !m_typing; break; }
			case sf::Keyboard::Delete: {deleteEntity(); break; }
			case sf::Keyboard::Enter: {
				if (m_fileName != "")
				{
					saveLevel();
				}
				else if (m_hasMenu)
				{
					if (m_addEntity)
					{
						addEntity();
					}
					else
					{
						changeAnimation();
					}
				}
				break;
				}
			}
		}

		if (m_typing && event.type == sf::Event::TextEntered)
		{
			if (event.text.unicode < 128)
			{
				sDrawFileName(event.text.unicode);
			}

		}

		if (event.type == sf::Event::KeyReleased)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::W: { m_player->getComponent<CInput>().up = false; break; }
			case sf::Keyboard::S: { m_player->getComponent<CInput>().down = false; break; }
			case sf::Keyboard::A: { m_player->getComponent<CInput>().left = false; break; }
			case sf::Keyboard::D: { m_player->getComponent<CInput>().right = false; break; }
			case sf::Keyboard::Space: { m_player->getComponent<CInput>().shoot = false; break; }
			}
		}
		if (event.type == sf::Event::MouseButtonPressed)
		{
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				setEntityDraggable(event.mouseButton.x, event.mouseButton.y);
			}

			if (event.mouseButton.button == sf::Mouse::Right)
			{

			}
		}
	}
}

void GameState_Play::sAnimation()
{
	// A string that builds the animation name
	std::string playerAnimation = "";

	//Set the action part of the animation
	if (m_player->getComponent<CTransform>().speed.x != 0 || m_player->getComponent<CTransform>().speed.y != 0)
	{
		playerAnimation += "Run";
	}
	else
	{
		playerAnimation += "Stand";
	}

	//Set the direction of the animation
	if (m_player->getComponent<CTransform>().facing == Vec2(0, 1))
	{
		playerAnimation += "Down";
	}
	if (m_player->getComponent<CTransform>().facing == Vec2(0, -1))
	{
		playerAnimation += "Up";
	}
	if (m_player->getComponent<CTransform>().facing == Vec2(1, 0))
	{
		playerAnimation += "Right";
		m_player->getComponent<CTransform>().scale.x = 1;
	}
	if (m_player->getComponent<CTransform>().facing == Vec2(-1, 0))
	{
		playerAnimation += "Right";
		m_player->getComponent<CTransform>().scale.x = -1;
	}

	//Update the animation of the player if it changed
	if (m_player->getComponent<CAnimation>().animation.getName() != playerAnimation)
	{
		m_player->getComponent<CAnimation>().animation = m_game.getAssets().getAnimation(playerAnimation);
	}

	//update the enemies animations
	for (auto& i : m_entityManager.getEntities("npc"))
	{
		// A string that builds the animation name
		std::string enemyAnimation = "";
		if (i->getComponent<CAnimation>().animation.getEntityName() == "Enemy1")
		{
			enemyAnimation += "En1";
		}
		else if (i->getComponent<CAnimation>().animation.getEntityName() == "Enemy2")
		{
			enemyAnimation += "En2";
		}
		else
		{
			enemyAnimation += "Boss";
		}

		if (i->getComponent<CTransform>().facing.x == 0)
		{
			enemyAnimation += "Jump";
		}
		else
		{
			enemyAnimation += "Walk";
			if (i->getComponent<CTransform>().facing.x < 0)
			{
				i->getComponent<CTransform>().scale.x = -1;
			}
			else
			{
				i->getComponent<CTransform>().scale.x = 1;
			}
		}
		//Update the animation of the enemy if it changed
		if (i->getComponent<CAnimation>().animation.getName() != enemyAnimation)
		{
			i->getComponent<CAnimation>().animation = m_game.getAssets().getAnimation(enemyAnimation);
		}
	}

	// Implement updating and setting of all animations here
	for (auto i : m_entityManager.getEntities())
	{
		if (i->hasComponent<CAnimation>())
		{
			if (i->getComponent<CAnimation>().repeat == true)
				i->getComponent<CAnimation>().animation.update();
			else
			{
				if (!i->getComponent<CAnimation>().animation.hasEnded())
					i->getComponent<CAnimation>().animation.update();
				else
				{
					i->destroy();
				}
			}
		}
	}
}

void GameState_Play::sCamera()
{
	sf::View view = m_game.window().getView();
	auto& p = m_player->getComponent<CTransform>().pos;
	if (m_follow)
	{
		// set the camera to follow the player

		view.setCenter(p.x, p.y);
		m_game.window().setView(view);
	}
	else
	{
		// Fix
		//Vec2(x * TX + RX * float(m_game.window().getSize().x) + x / 2, y * TY + RY * float(m_game.window().getSize().y) + y / 2));
		// took idea from this on how to change room view
		float RX = std::floor(p.x / m_game.window().getSize().x);
		float RY = std::floor(p.y / m_game.window().getSize().y);
		float height = std::floor(m_game.window().getSize().y);
		float width = std::floor(m_game.window().getSize().x);

		view.setCenter((width * RX) + width / 2, (height * RY) + height / 2);
		m_game.window().setView(view);
	}
}

void GameState_Play::sRender()
{
	m_game.window().clear(sf::Color(0, 0, 0));

	// draw all Entity textures / animations
	if (m_drawTextures)
	{
		for (auto e : m_entityManager.getEntities())
		{
			auto& transform = e->getComponent<CTransform>();

			if (e->hasComponent<CAnimation>())
			{
				auto& animation = e->getComponent<CAnimation>().animation;
				animation.getSprite().setRotation(transform.angle);
				animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
				animation.getSprite().setScale(transform.scale.x, transform.scale.y);
				if (animation.getEntityName() == "Water") {
					shader.setUniform("currentTexture", sf::Shader::CurrentTexture);
					m_game.window().draw(animation.getSprite(), &shader);
				}
				else if (animation.getEntityName() == "Rock")
				{
					shader1.setUniform("currentTexture", sf::Shader::CurrentTexture);
					m_game.window().draw(animation.getSprite(), &shader1);
				}
				else if (animation.getEntityName() == "Cave")
				{
					m_game.window().draw(animation.getSprite(), &shader2);
				}
				else
				{
					m_game.window().draw(animation.getSprite());
				}
			}
		}
	}

	// draw all Entity collision bounding boxes with a rectangleshape
	if (m_drawCollision)
	{
		sf::CircleShape dot(4);
		dot.setFillColor(sf::Color::Black);
		for (auto e : m_entityManager.getEntities())
		{
			if (e->hasComponent<CBoundingBox>())
			{
				auto& box = e->getComponent<CBoundingBox>();
				auto& transform = e->getComponent<CTransform>();
				sf::RectangleShape rect;
				rect.setSize(sf::Vector2f(box.size.x - 1, box.size.y - 1));
				rect.setOrigin(sf::Vector2f(box.halfSize.x, box.halfSize.y));
				rect.setPosition(transform.pos.x, transform.pos.y);
				rect.setFillColor(sf::Color(0, 0, 0, 0));

				if (box.blockMove && box.blockVision) { rect.setOutlineColor(sf::Color::Black); }
				if (box.blockMove && !box.blockVision) { rect.setOutlineColor(sf::Color::Blue); }
				if (!box.blockMove && box.blockVision) { rect.setOutlineColor(sf::Color::Red); }
				if (!box.blockMove && !box.blockVision) { rect.setOutlineColor(sf::Color::White); }
				rect.setOutlineThickness(1);
				m_game.window().draw(rect);
			}

			if (e->hasComponent<CPatrol>())
			{
				auto& patrol = e->getComponent<CPatrol>().positions;
				for (size_t p = 0; p < patrol.size(); p++)
				{
					dot.setPosition(patrol[p].x, patrol[p].y);
					m_game.window().draw(dot);
				}
			}

			if (e->hasComponent<CFollowPlayer>())
			{
				sf::VertexArray lines(sf::LinesStrip, 2);
				lines[0].position.x = e->getComponent<CTransform>().pos.x;
				lines[0].position.y = e->getComponent<CTransform>().pos.y;
				lines[0].color = sf::Color::Black;
				lines[1].position.x = m_player->getComponent<CTransform>().pos.x;
				lines[1].position.y = m_player->getComponent<CTransform>().pos.y;
				lines[1].color = sf::Color::Black;
				m_game.window().draw(lines);
				dot.setPosition(e->getComponent<CFollowPlayer>().home.x, e->getComponent<CFollowPlayer>().home.y);
				m_game.window().draw(dot);
			}
		}
	}
	m_game.window().draw(m_fileNameToDraw);

	if (m_snapToGrid)
	{
		sDrawGrid();
	}

	if (m_hasMenu)
	{
		size_t halfMenuRect = 42;
		int xMenuPos = m_game.window().getView().getCenter().x + (m_game.window().getView().getSize().x / 2) - halfMenuRect;
		int yAdjust = m_game.window().getView().getCenter().y - (m_game.window().getView().getSize().y / 2);
		size_t menuColumns = ceil(m_menuAnimations.size() / 12.0);
		sf::RectangleShape menuRect;
		menuRect.setSize(sf::Vector2f((halfMenuRect * 2) * menuColumns, m_game.window().getSize().y));
		menuRect.setOrigin(sf::Vector2f(halfMenuRect * menuColumns, m_game.window().getSize().y / 2));
		menuRect.setPosition(xMenuPos, m_game.window().getView().getCenter().y);
		sf::Color rectColor = sf::Color::Black;
		rectColor.a = 128;
		menuRect.setFillColor(rectColor);
		m_game.window().draw(menuRect);
		
		for (int i = 0; i < m_menuAnimations.size(); i++)
		{
			m_menuAnimations[i].getSprite().setPosition(xMenuPos, int((halfMenuRect * 2) * i + halfMenuRect) + yAdjust);
			m_game.window().draw(m_menuAnimations[i].getSprite());
		}

		sf::RectangleShape selectionRect;
		selectionRect.setSize(sf::Vector2f(halfMenuRect * 2, halfMenuRect * 2));
		selectionRect.setOrigin(sf::Vector2f(halfMenuRect, halfMenuRect));
		selectionRect.setPosition(xMenuPos, int((halfMenuRect * 2) * m_menuIndex + halfMenuRect) + yAdjust);
		selectionRect.setFillColor(sf::Color(0, 0, 0, 0));
		selectionRect.setOutlineColor(sf::Color::White);
		selectionRect.setOutlineThickness(5);
		m_game.window().draw(selectionRect);
	}

	m_game.window().display();
}

void GameState_Play::deleteEntity()
{
	// delete the currently dragged entity
	auto& entities = m_entityManager.getEntities();
	for (auto e : entities)
	{
		if (e->getComponent<CDrag>().drag)
		{
			e->destroy();
			return;
		}
	}
}

void GameState_Play::initializeAddMenu()
{
	auto entityNames = m_game.getAssets().getEntityNames();
	for (auto entity : entityNames)
	{
		auto animations = m_game.getAssets().getEntityAnimation(entity);
		for (auto entityAnimation : animations)
		{
			if (entityAnimation.isAddAnimation())
			{
				m_menuAnimations.push_back(entityAnimation);
			}
		}
	}
	m_menuIndex = 0;
	m_hasMenu = true;
	m_addEntity = true;
}

void GameState_Play::initializeChangeAnimationMenu()
{
	// put down an entity that is currently being dragged
	auto& entities = m_entityManager.getEntities();
	for (auto e : entities)
	{
		if (e->getComponent<CDrag>().drag)
		{
			m_changeAnimation = e;
		}
	}

	// if no entity is being dragged then exit
	if (!m_changeAnimation)
	{
		return;
	}


	auto animations = m_game.getAssets().getEntityAnimation(m_changeAnimation->getComponent<CAnimation>().animation.getEntityName());
	for (auto entityAnimation : animations)
	{
		m_menuAnimations.push_back(entityAnimation);
	}
	m_menuIndex = 0;
	m_hasMenu = true;
	m_addEntity = false;
}

void GameState_Play::addEntity()
{
	//get the selected animation and clear the menu
	auto selectedAnimation = m_menuAnimations[m_menuIndex];
	m_menuAnimations.clear();

	// put down an entity that is currently being dragged
	auto& entities = m_entityManager.getEntities();
	for (auto e : entities)
	{
		if (e->getComponent<CDrag>().drag)
		{
			e->getComponent<CDrag>().drag = false;
		}
	}

	// initalize the entity fom an existing entity if possible
	for (auto e : entities)
	{
		if (e->getComponent<CAnimation>().animation.getName() == selectedAnimation.getName())
		{
			auto newEntity = m_entityManager.addEntity(e->tag());
			newEntity->addComponent<CAnimation>(selectedAnimation, e->getComponent<CAnimation>().repeat);
			Vec2 size = e->getComponent<CBoundingBox>().size;
			bool blockMove = e->getComponent<CBoundingBox>().blockMove;
			bool blockVision = e->getComponent<CBoundingBox>().blockVision;
			newEntity->addComponent<CBoundingBox>(size, blockMove, blockVision);
			newEntity->addComponent<CTransform>(Vec2(0, 0));
			newEntity->getComponent<CTransform>().prevPos = newEntity->getComponent<CTransform>().pos;
			newEntity->addComponent<CDrag>();
			newEntity->getComponent<CDrag>().drag = !newEntity->getComponent<CDrag>().drag;

			if (e->hasComponent<CFollowPlayer>() || e->hasComponent<CPatrol>())
			{
				int speed = 0;
				if (e->hasComponent<CFollowPlayer>())
				{
					speed = e->hasComponent<CFollowPlayer>();
				}
				else
				{
					speed = e->hasComponent<CPatrol>();
				}

				newEntity->addComponent<CFollowPlayer>(Vec2(int(sf::Mouse::getPosition(m_game.window()).x), int(sf::Mouse::getPosition(m_game.window()).y)), speed, false);
			}

			m_hasMenu = false;
			return;
		}

	}

	// if there is not existing entity, initialize using defaults
	auto newEntity = m_entityManager.addEntity("tile");
	newEntity->addComponent<CAnimation>(selectedAnimation, true);
	newEntity->addComponent<CBoundingBox>(Vec2(m_tileSize, m_tileSize), true, true);
	newEntity->addComponent<CTransform>(Vec2(0, 0));
	newEntity->getComponent<CTransform>().prevPos = newEntity->getComponent<CTransform>().pos;
	newEntity->addComponent<CDrag>();
	newEntity->getComponent<CDrag>().drag = !newEntity->getComponent<CDrag>().drag;

	m_hasMenu = false;
}

void GameState_Play::changeAnimation()
{
	//get the selected animation and clear the menu
	auto selectedAnimation = m_menuAnimations[m_menuIndex];
	m_menuAnimations.clear();

	m_changeAnimation->getComponent<CAnimation>().animation = selectedAnimation;
	m_changeAnimation.reset();

	m_hasMenu = false;
}
