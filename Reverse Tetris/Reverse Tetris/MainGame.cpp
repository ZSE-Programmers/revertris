#include "MainGame.h"
#include <iostream>
#include <random>
#include <ctime>

MainGame::MainGame()
{
	m_gameState = GameState::PLAY;
}


MainGame::~MainGame()
{
	
}

void MainGame::Run()
{
	InitSystems();
	GameLoop();
}

void MainGame::InitSystems()
{
	// Creating window and getting renderer
	m_window.Create(800, 600);
	m_renderer = m_window.GetRenderer();

	m_levelData = m_level.GetLevelData();

	// Initalizing textures
	m_levelTexture = m_sprite.LoadTexture("Textures/bgtext.jpg", m_renderer);
	m_blueSquare = m_sprite.LoadTexture("Textures/element_blue_square.png", m_renderer);
	m_redSquare = m_sprite.LoadTexture("Textures/element_red_square.png", m_renderer);
	m_greenSquare = m_sprite.LoadTexture("Textures/element_green_square.png", m_renderer);
	m_purpleSquare = m_sprite.LoadTexture("Textures/element_purple_cube_glossy.png", m_renderer);
	m_yellowSquare = m_sprite.LoadTexture("Textures/element_yellow_square.png", m_renderer);
	m_pinkSquare = m_sprite.LoadTexture("Textures/pink_square.png", m_renderer);

	InitBlocks();
	InitLevel();
}

void MainGame::GameLoop()
{
	while (m_gameState != GameState::EXIT)
	{
		ProcessInput();
		Update();
		Draw();
	}
}

void MainGame::Draw()
{
	SDL_RenderClear(m_renderer);
	
	// Draw level
	for (int y = 0; y < m_levelData.size(); y++)
	{
		for (int x = 0; x < m_levelData[y].size(); x++)
		{
			SDL_Rect destRect = { x * TILE_WIDTH, y * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH };
			char tile = m_levelData[y][x];
			switch (tile)
			{
			case '#':
				SDL_RenderCopy(m_renderer, m_levelTexture, NULL, &destRect);
				break;
			case '1':
				SDL_RenderCopy(m_renderer, m_blueSquare, NULL, &destRect);
				break;
			case '2':
				SDL_RenderCopy(m_renderer, m_redSquare, NULL, &destRect);
				break;
			case '3':
				SDL_RenderCopy(m_renderer, m_greenSquare, NULL, &destRect);
				break;
			case '4':
				SDL_RenderCopy(m_renderer, m_purpleSquare, NULL, &destRect);
				break;
			case '5':
				SDL_RenderCopy(m_renderer, m_yellowSquare, NULL, &destRect);
				break;
			case '6':
				SDL_RenderCopy(m_renderer, m_greenSquare, NULL, &destRect);
				break;
			case '7':
				SDL_RenderCopy(m_renderer, m_greenSquare, NULL, &destRect);
				break;
			}
		}
	}
	SDL_RenderPresent(m_renderer);
}

void MainGame::Update()
{
	if (m_inputManager.IsButtonPressed())
	{
		RemoveBlock();
	}
	if (m_stackQueue.size() < 3)
	{
		UpdateQueue();
	}
}

void MainGame::ProcessInput()
{
	SDL_Event tmp_event;
	while (SDL_PollEvent(&tmp_event))
	{
		switch (tmp_event.type)
		{
		case SDL_QUIT:
			m_gameState = GameState::EXIT;
			exit(1);
			break;
		case SDL_MOUSEMOTION:
			m_inputManager.SetMousePosition(glm::vec2(tmp_event.motion.x, tmp_event.motion.y));
			break;
		case SDL_MOUSEBUTTONDOWN:
			m_inputManager.PressButton();
			break;
		case SDL_MOUSEBUTTONUP:
			m_inputManager.ReleaseButton();
			break;
		case SDL_KEYDOWN:
			m_inputManager.PressKey(tmp_event.key.keysym.sym);
			break;
		case SDL_KEYUP:
			m_inputManager.ReleaseKey(tmp_event.key.keysym.sym);
			break;
		}
	}
}

void MainGame::InitBlocks()
{
	LShape.Init("Levels/LShape.txt", 1);
	RLShape.Init("Levels/RLShape.txt", 2);
	ZShape.Init("Levels/ZShape.txt", 3);
	RZShape.Init("Levels/RZShape.txt", 4);
	SQShape.Init("Levels/SQShape.txt", 5);

	ZShape2.Init("Levels/ZShape2.txt", 3);
	RZShape2.Init("Levels/RZShape2.txt", 4);
}

void MainGame::InitLevel()
{
	int counter = 10;
	for (int y = m_levelData.size() - 1; y >= 0; y--)
	{
		for (int x = m_levelData[y].size() - 1; x >= 0; x--)
		{
			char tile = m_levelData[y][x];
			if (tile == '.')
			{
				if (InsertBlock(x, y))
				{
					counter--;
					if (counter == 0)
					{
						return;
					}
				}
			}
		}
	}
}

bool MainGame::InsertBlock(int x, int y)
{
	// Count pieces of block are fitting
	int counter = 0;

	// Rolling which block have to be fit
	static std::mt19937 randomEngine(time(NULL));
	static std::uniform_int_distribution <int> rollShape(1, 4);
	int roll = rollShape(randomEngine);
	
	// Vector for holding shape
	std::vector <std::string> shape;

	// Processing which which shape to draw
	switch (roll)
	{
	case 1:
		shape = RZShape.GetShape();
		if (CanPlaceBlock(x, y, shape))
		{
			m_blockTypes.push_back(RZShape);
			m_blocks.back()->AddType(3);
			return true;
		}
		break;
	case 2:
		shape = ZShape.GetShape();
		if (CanPlaceBlock(x, y, shape))
		{
			m_blockTypes.push_back(ZShape);
			m_blocks.back()->AddType(4);
			return true;
		}
		break;
	case 3:
		shape = ZShape2.GetShape();
		if (CanPlaceBlock(x, y, shape))
		{	
			m_blockTypes.push_back(ZShape2);
			m_blocks.back()->AddType(4);
			return true;
		}
		break;
	case 4:
		shape = RZShape2.GetShape();
		if (CanPlaceBlock(x, y, shape))
		{
			m_blockTypes.push_back(RZShape2);
			m_blocks.back()->AddType(3);
			return true;
		}
		break;
	}
	return false;
}

void MainGame::RemoveBlock()
{
	int counter = 0;
	glm::vec2 mousePosition = m_inputManager.GetMousePosition();
	// Converting mouse screen coords to levelData coords
	mousePosition = glm::vec2(floor(mousePosition.x / TILE_WIDTH), floor(mousePosition.y / TILE_WIDTH));

	// Check if this is block or just a part of map
	if (m_levelData[mousePosition.y][mousePosition.x] != '.' &&
		m_levelData[mousePosition.y][mousePosition.x] != '-' &&
		m_levelData[mousePosition.y][mousePosition.x] != '0' &&
		m_levelData[mousePosition.y][mousePosition.x] != '#')
	{
		// Return index to block we have to remove
		int index = -1;
		index = FindBlock(mousePosition);
		if (index != -1)
		{
			// Check if we can remove block
			if (m_blocks[index]->CanRemove(m_levelData))
			{
				std::vector <glm::vec2> squarePosition = m_blocks[index]->GetPosition();
				delete m_blocks[index];
				m_blocks[index] = m_blocks.back();
				m_blocks.pop_back();
				for (int i = 0; i < squarePosition.size(); i++)
				{
					m_levelData[squarePosition[i].y][squarePosition[i].x] = '.';
				}
			}
		}
	}
}

int MainGame::FindBlock(glm::vec2 position)
{
	for (int i = 0; i < m_blocks.size(); i++)
	{
		std::vector <glm::vec2> blockPosition = m_blocks[i]->GetPosition();

		for (int j = 0; j < blockPosition.size(); j++)
		{
			if (blockPosition[j] == position)
			{
				return i;
			}
		}
	}
}

bool MainGame::CanPlaceBlock(int x, int y, std::vector<std::string>& shape)
{
	int counter = 0;
	// Check all tiles from shape 
	for (int i = 0; i < shape.size(); i++)
	{
		for (int j = 0; j < shape[i].size(); j++)
		{
			char tile = shape[i][j];
			if (tile != '.')
			{
				if (m_levelData[y + i][x + j] == '.')
				{
					counter++;
				}
			}
		}
	}

	// If all 4 tiles are empty we can draw our block
	if (counter == 4)
	{
		char tile;
		Block* tmp_block = new Block;
		for (int i = 0; i < shape.size(); i++)
		{
			for (int j = 0; j < shape[i].size(); j++)
			{
				tile = shape[i][j];
				if (tile != '.')
				{
					m_levelData[y + i][x + j] = tile;
					tmp_block->AddSquare(x + j, y + i);
				}
			}
		}
		m_blocks.push_back(tmp_block);
		return true;
	}
	return false;
}

void MainGame::UpdateQueue()
{
	int counter = 0;
	if (!m_blockTypes.empty())
	{
		for (int i = 0; i < m_maxStackSize; i++)
		{
			std::mt19937 randomEngine(time(NULL));
			std::uniform_int_distribution <int> roll(0, m_blockTypes.size() - 1);

			m_stackQueue.push_back(m_blockTypes[roll(randomEngine)]);
		}

		for (int i = 0; i < m_levelData.size(); i++)
		{
			for (int j = 0; j < m_levelData[i].size(); j++)
			{
				if (m_levelData[i][j] == '-')
				{
					std::vector <std::string> shape = m_stackQueue.back().GetShape();
					if (CanPlaceQueue(shape, j, i))
					{
						counter++;
						if (counter >= m_maxStackSize)
						{
							return;
						}
					}
				}
			}
		}
	}
}

bool MainGame::CanPlaceQueue(std::vector<std::string>& shape, int x, int y)
{
	for (int i = 0; i < shape.size(); i++)
	{
		for (int j = 0; j < shape[i].size(); j++)
		{
			if (shape[i][j] != '.')
			{
				m_levelData[y + i][x + j] = shape[i][j];
			}
		}
	}
	return true;
}

