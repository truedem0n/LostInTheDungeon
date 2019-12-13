#include "GameState_Menu.h"
#include "GameState_Play.h"
#include "Common.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"

GameState_Menu::GameState_Menu(GameEngine & game)
    : GameState(game)
{
    init("");
}

void GameState_Menu::init(const std::string & menuConfig)
{

	if (!m_music.openFromFile("sounds/mainMenuMusic.wav"))
		std::cout << "error loading sound file"; // error
	m_music.setLoop(true);
	if (!m_buffer.loadFromFile("sounds/selection.wav"))
		std::cout << "error loading sound file"; // error
	m_sound.setBuffer(m_buffer);




    m_title = "Lost in the dungeon";
	m_menuStrings.push_back("Level  1");
	m_menuStrings.push_back("Level  2");
	m_menuStrings.push_back("Level  3");
	m_menuStrings.push_back("Final Level");

	m_levelPaths.push_back("level1.txt");
	m_levelPaths.push_back("level2.txt");
	m_levelPaths.push_back("level3.txt");
	//m_levelPaths.push_back("studentlevel.txt");

    m_menuText.setFont(m_game.getAssets().getFont("PeicesNfi"));
    m_menuText.setCharacterSize(128);
}

void GameState_Menu::update()
{
	playMusic();
    m_entityManager.update();
    sUserInput();
    sRender();
}

void GameState_Menu::playMusic()
{
	if (m_inMainMenu)
	{
		m_music.play();
		m_music.setLoop(true);
		m_music.setVolume(50);
		m_inMainMenu = false;
	}
}

void GameState_Menu::sUserInput()
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
                case sf::Keyboard::Escape: 
                { 
                    m_game.quit(); 
                    break; 
                }
                case sf::Keyboard::W: 
                {
					m_sound.play();
                    if (m_selectedMenuIndex > 0) { m_selectedMenuIndex--; }
                    else { m_selectedMenuIndex = m_menuStrings.size() - 1; }
                    break;
                }
                case sf::Keyboard::S: 
                { 
					m_sound.play();
                    m_selectedMenuIndex = (m_selectedMenuIndex + 1) % m_menuStrings.size(); 
                    break; 
                }
                case sf::Keyboard::D: 
                { 
					m_music.stop();
                    m_game.pushState(std::make_shared<GameState_Play>(m_game, m_levelPaths[m_selectedMenuIndex]));
					m_inMainMenu = true;
                    break; 
                }
				case sf::Keyboard::Q:
				{
					if (m_music.getStatus() == m_music.Stopped)
						m_music.play();
					else
						m_music.stop();
					break;
				}
            }
        }
    }

}

void GameState_Menu::sRender()
{

    // clear the window to a black
    m_game.window().setView(m_game.window().getDefaultView());
    m_game.window().clear(sf::Color(0, 0, 0));


	for (size_t i = 0; i < m_menuStrings.size(); i++)
	{
		m_menuText.setCharacterSize(30);
		m_menuText.setFillColor(sf::Color(100, 0, 0));
		char c = m_charGenerator.at(0 + (std::rand() % (m_charGenerator.length() - 1 - 0 + 1)));
		m_menuText.setString(c);
		m_menuText.setPosition(sf::Vector2f(0 + (std::rand() % (m_game.window().getSize().x - 0 + 1)), 0 + (std::rand() % (m_game.window().getSize().y - 0 + 1))));
		m_menuText.setFont(m_game.getAssets().getFont("Splat"));
		m_game.window().draw(m_menuText);
	}

    // draw the game title in the top-left of the screen
	m_menuText.setFont(m_game.getAssets().getFont("PeicesNfi"));
    m_menuText.setCharacterSize(60);
    m_menuText.setString(m_title);
    m_menuText.setFillColor(sf::Color::Red);
    m_menuText.setPosition(sf::Vector2f(m_game.window().getSize().x/4, 100));
    m_game.window().draw(m_menuText);
    
    // draw all of the menu options
    for (size_t i = 0; i < m_menuStrings.size(); i++)
    {
        m_menuText.setString(m_menuStrings[i]);
        m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::Red : sf::Color(100, 0, 0));
        m_menuText.setPosition(sf::Vector2f(m_game.window().getSize().x / 2.5, 200 + i * 80));
        m_game.window().draw(m_menuText);
    }

    // draw the controls in the bottom-left
    m_menuText.setCharacterSize(30);
    m_menuText.setFillColor(sf::Color(100, 0, 0));
    m_menuText.setString("up: w     down: s    play: d      back: esc      q: music");
    m_menuText.setPosition(sf::Vector2f(10, 730));
    m_game.window().draw(m_menuText);

	
	

    m_game.window().display();
}