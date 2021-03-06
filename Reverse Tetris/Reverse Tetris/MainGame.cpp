#include "MainGame.h"
#include <iostream>
#include <random>
#include <ctime>
#include <fstream>

MainGame::MainGame() : m_gameState(GameState::TUTORIAL), m_newLines(0), m_speed(2.0f), m_font(nullptr), m_score(0), m_multiplier(0.95f), m_tutorialUpdate(false), 
m_music(nullptr), m_destroy(nullptr), m_renderer(nullptr)
{
    // Initalizing font
    TTF_Init();
    m_font = TTF_OpenFont("Fonts/BEBAS.ttf", 32);
    if (m_font == nullptr)
    {
        std::cout << "Could not load font" << std::endl;
    }
}

MainGame::~MainGame()
{
    for (int i = 0; i < m_blocks.size(); i++)
    {
        delete m_blocks[i];
    }
    for (int i = 0; i < m_newBlocks.size(); i++)
    {
        delete m_newBlocks[i];
    }
}

void MainGame::Run()
{
    InitSystems();
    GameLoop();
    PrintEndscreen();
}

void MainGame::InitSystems()
{
    // Creating window and getting renderer
    m_window.Create(800, 600);
    m_renderer = m_window.GetRenderer();

    m_levelData = m_level.GetLevelData();

    // Initalizing textures
    m_levelTexture = m_sprite.LoadTexture("Textures/mountain.png", m_renderer);
    m_blueSquare = m_sprite.LoadTexture("Textures/element_blue_square.png", m_renderer);
    m_redSquare = m_sprite.LoadTexture("Textures/element_red_square.png", m_renderer);
    m_greenSquare = m_sprite.LoadTexture("Textures/element_green_square.png", m_renderer);
    m_purpleSquare = m_sprite.LoadTexture("Textures/element_purple_cube_glossy.png", m_renderer);
    m_yellowSquare = m_sprite.LoadTexture("Textures/element_yellow_square.png", m_renderer);
    m_pinkSquare = m_sprite.LoadTexture("Textures/white-gray.png", m_renderer);
    m_graySquare = m_sprite.LoadTexture("Textures/black.png", m_renderer);
    m_border = m_sprite.LoadTexture("Textures/border_horizontal.png", m_renderer);
    m_fingerTexture = m_sprite.LoadTexture("Textures/finger.png", m_renderer);

    //Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    }
    //Load music
    m_music = Mix_LoadMUS("Music/BoxCat_Games_-_18_-_Melody.mp3");
    if (m_music == nullptr)
    {
        printf("Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError());
    }
    m_destroy = Mix_LoadWAV("Music/collide_wall.wav");
    m_fast = Mix_LoadWAV("Music/G14F.wav");
    m_impressive = Mix_LoadWAV("Music/G13F.wav");

    // Start background music
    Mix_PlayMusic(m_music, -1);

    // Initalize and play tutorial 
    InitTutorial();

    // Loading first blocks on map
    InitLevel();

    // Initalizing queue
    InitQueue();

    // Initalizing new blocks
    InitNewBlocks();
}

void MainGame::GameLoop()
{
    DrawQueue(m_levelData);
    while (m_gameState == GameState::PLAY)
    {
        SDL_RenderClear(m_renderer);

        m_fps.Start();

        ProcessInput();

        // Processing remove blocks, roll new blocks and check for lose
        Update();

        // Updating blocks, moving them up
        UpdateBlocks();

        // Rendering level and blocks
        Draw(m_levelData);

        // Render player score
        m_window.RenderScore(m_score, m_font, m_renderer, m_textTexture, m_tutorialTime);

        float fps = m_fps.End();
        //std::cout << fps << std::endl;

        SDL_RenderPresent(m_renderer);
    }
}

void MainGame::Draw(std::vector <std::string>& data)
{
    // Draw level
    for (int y = 0; y < data.size(); y++)
    {
        for (int x = 0; x < data[y].size(); x++)
        {
            SDL_Rect destRect = { x * TILE_WIDTH, y * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH };
            char tile = data[y][x];
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
                SDL_RenderCopy(m_renderer, m_pinkSquare, NULL, &destRect);
                break;
            case '7':
                SDL_RenderCopy(m_renderer, m_graySquare, NULL, &destRect);
                break;
            case 'm': // Finger tile
                destRect.w = 100;
                destRect.h = 100;
                SDL_RenderCopy(m_renderer, m_fingerTexture, NULL, &destRect);
                break;
            }
        }
    }

    // Draw frames around blocks
    for (int i = 0; i < m_blocks.size(); i++)
    {
        m_blocks[i]->DrawFrame(m_renderer, m_border, data);
    }
}

void MainGame::Update()
{
    // If player click block process remove
    if (m_inputManager.IsButtonPressed())
    {
        RemoveBlock(m_levelData);
    }

    // Check if player lost
    for (int i = 1; i < m_levelData[2].size(); i++)
    {
        if (i < 14)
        {
            if (m_levelData[2][i] != '.')
            {
                m_gameState = GameState::EXIT;
            }
        }
    }

    // If new blocks txt file is empty we roll new blocks
    if (m_newLines == m_newBlocksData.size() - 4)
    {
        m_newBlocksData.clear();
        InitNewBlocks();
        m_newLines = 0;
    }
}

void MainGame::UpdateBlocks()
{
    // Moving all blocks one tile up
    if ((SDL_GetTicks() / 1000.0f)  - m_tutorialTime > m_speed)
    {
        // Blocks moves faster and faster
        m_speed += (2.0f * m_multiplier);

        // Moving blocks one tile up
        for (int y = 0; y < m_levelData.size(); y++)
        {
            for (int x = 0; x < m_levelData[y].size(); x++)
            {
                char tmp_tile = m_levelData[y][x];
                if (tmp_tile != '#' && tmp_tile != 'x' && tmp_tile != '-' && y != 1 && y != 16 && x < 14)
                {
                    m_levelData[y][x] = m_levelData[y + 1][x];
                }
                // Last lane
                else if (y == 16)
                {
                    m_levelData[y] = m_newBlocksData[m_newLines];
                }
            }
        }
        MoveUp();
        AddNewBlocks(m_newLines);
        m_newLines++;
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

void MainGame::InitLevel()
{
    // How many blocks we want to spawn at the begining
    int counter = 10;

    // Looping trough level data from end
    for (int y = m_levelData.size() - 1; y >= 0; y--)
    {
        for (int x = m_levelData[y].size() - 1; x >= 0; x--)
        {
            if (InitBlocks({ x, y }))
            {
                counter--;
                if (counter <= 0)
                {
                      // This means we are done
                     return;
                }
            }
        }
    }
}

void MainGame::PrintEndscreen()
{
    Mix_PauseMusic();
    SDL_RenderClear(m_renderer);

    // Initalizing score texture
    std::string str = "You  earned  " + std::to_string(m_score) + "  points!";
    SDL_Color textColor = { 255, 255, 255, 255 };
    SDL_Surface* textSurface = TTF_RenderText_Solid(m_font, str.c_str(), textColor);
    if (textSurface == NULL)
    {
        std::cout << "Unable to create text surface!" << std::endl;
    }
    else
    {
        m_tapHereTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
        if (m_tapHereTexture == NULL)
        {
            std::cout << "Unable to create text texture!\n";
        }

        SDL_FreeSurface(textSurface);
    }
    SDL_Rect destRect = { 70, 100, 700, 300 };
    SDL_RenderCopy(m_renderer, m_tapHereTexture, NULL, &destRect);
    SDL_RenderPresent(m_renderer);

    if (m_score < 100)
    {
        Mix_PlayChannel(-1, m_fast, 0);
    }
    else
    {
        Mix_PlayChannel(-1, m_impressive, 0);
    }
}

void MainGame::InitTutorial()
{
    // Opening log file
    std::ifstream file;
    std::string input;
    file.open("log.txt");
    if (file.fail())
    {
        perror("Could not open log.txt!");
    }

    // Getting how many times we ran game
    file >> input;
    int runs;
    runs = atoi(input.c_str());

    // If we played more less then 3 times we play tutorial
    if (runs < 3)
    {
        // Initalizing tutorial data
        m_level.InitTutorial("Levels/tutorial1.txt", m_blocks, m_blockTypes, 0);
        InitQueue();

        // Initalizing tap here texture
        SDL_Color textColor = { 255, 255, 255, 255 };
        SDL_Surface* textSurface = TTF_RenderText_Solid(m_font, "Tap here", textColor);
        if (textSurface == NULL)
        {
            std::cout << "Unable to create text surface!" << std::endl;
        }
        else
        {
            m_tapHereTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
            if (m_tapHereTexture == NULL)
            {
                std::cout << "Unable to create text texture!\n";
            }

            SDL_FreeSurface(textSurface);
        }
        SDL_Rect destRect = { 340, 345, 100, 50 };

        // First part of tutorial
        PlayTutorial(destRect, 0);
        std::cout << runs << std::endl;

        // Initalizing texture for second part
        textColor = { 255, 255, 255, 255 };
        textSurface = TTF_RenderText_Solid(m_font, "You can't tap this cuz other lay on him", textColor);
        if (textSurface == NULL)
        {
            std::cout << "Unable to create text surface!" << std::endl;
        }
        else
        {
            m_tapHereTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
            if (m_tapHereTexture == NULL)
            {
                std::cout << "Unable to create text texture!\n";
            }

            SDL_FreeSurface(textSurface);
        }
        destRect = { 40, 335, 400, 50 };

        // Second part of tutorial
        m_gameState = GameState::TUTORIAL;
        m_tutorialData.clear();
        m_level.InitTutorial("Levels/tutorial2.txt", m_blocks, m_blockTypes, 1);
        m_tutorialData = m_level.GetTutorialData();
        InitQueue();
        PlayTutorial(destRect, 1);

        m_score = 0;
    }
    else
    {
        m_gameState = GameState::PLAY;
    }
    // Increasing runs and closing input file
    runs++;
    file.clear();
    file.close();

    // Opening output file and saving new value of runs
    std::ofstream file2;
    file2.open("log.txt");
    file2 << runs;
    file2.close();
}

void MainGame::PlayTutorial(SDL_Rect& destRect, int part)
{
    m_tutorialData = m_level.GetTutorialData();
    DrawQueue(m_tutorialData);

    while (m_gameState == GameState::TUTORIAL)
    {
        SDL_RenderClear(m_renderer);

        m_fps.Start();

        ProcessInput();

        // Processing remove blocks, roll new blocks and check for lose
        if (UpdateTutorial(destRect))
        {
        }
        
        if (part == 0)
        {
            if (m_tutorialUpdate != false)
            {
                for (int y = 0; y < m_tutorialData.size(); y++)
                {
                    for (int x = 0; x < m_tutorialData[y].size(); x++)
                    {
                        if (m_tutorialData[y][x] == 'm')
                        {
                            m_tutorialData[y][x] = '.';
                            m_tutorialData[y][x - 2] = 'm';
                            destRect.x -= 64;
                            m_tutorialUpdate = false;
                        }
                    }
                }
            }
        }


        SDL_RenderCopy(m_renderer, m_tapHereTexture, NULL, &destRect);

        // Rendering level and blocks
        Draw(m_tutorialData);

        float fps = m_fps.End();
        //std::cout << fps << std::endl;

        m_tutorialTime = SDL_GetTicks() / 1000.0f;
        //std::cout << m_tutorialTime << std::endl;

        SDL_RenderPresent(m_renderer);
    }
}

bool MainGame::UpdateTutorial(SDL_Rect& destRect)
{
    // If player click block process remove
    if (m_inputManager.IsButtonPressed())
    {
        RemoveBlock(m_tutorialData);
        return true;
    }

    if (m_blocks.empty())
    {
        m_blocks.clear();
        m_blockTypes.clear();
        m_stackQueue.clear();
        m_gameState = GameState::PLAY;
        return false;
    }
    return false;
}

bool MainGame::InitBlocks(glm::vec2 position)
{
    if (m_levelData[position.y][position.x] == '.')
    {
        if (m_level.InsertBlock(position, m_levelData, m_blocks, true, m_blockTypes))
        {
            return true;
        }
    }
    return false;
}

void MainGame::RemoveBlock(std::vector <std::string>& data)
{
    glm::vec2 mousePosition = m_inputManager.GetMousePosition();
    // Converting mouse screen coords to levelData coords
    mousePosition = glm::vec2(floor(mousePosition.x / TILE_WIDTH), floor(mousePosition.y / TILE_WIDTH));

    // Check if this is block or just a part of map
    if (data[mousePosition.y][mousePosition.x] != '.' &&
    mousePosition.x > 0 && mousePosition.x < 14 && mousePosition.y > 0 && mousePosition.y < 17)
    {
        // Return index to block we have to remove
        int index = FindBlock(mousePosition, m_blocks);
        if (index != -1)
        {
            if (m_stackQueue.front().GetType() == m_blocks[index]->GetShape().GetType())
            {
                ProcessRemove(index, data);
                return;
            }
        }
    }
}

void MainGame::ProcessRemove(int index, std::vector <std::string>& data)
{
    // Check if we can remove block
    if (m_blocks[index]->CanRemove(data))
    {
        // Adding points to player score
        m_score += 10;
        m_tutorialUpdate = true;
        Mix_PlayChannel(-1, m_destroy, 0);
        // Processing queue
        RemoveQueue(data);
        UpdateQueue();
        DrawQueue(data);

        // Position of squares that we can to replace by '.'
        std::vector <glm::ivec2> squarePosition = m_blocks[index]->GetPosition();
        // Deleting block
        delete m_blocks[index];
        m_blocks[index] = m_blocks.back();
        m_blocks.pop_back();
        // Replacing sqaures by '.'
        for (int i = 0; i < squarePosition.size(); i++)
        {
            data[squarePosition[i].y][squarePosition[i].x] = '.';
        }
    }
}

int MainGame::FindBlock(glm::ivec2 position, std::vector <Block*>& blocks)
{
    for (int i = 0; i < blocks.size(); i++)
    {
        std::vector <glm::ivec2> blockPosition = blocks[i]->GetPosition();
        for (int j = 0; j < blockPosition.size(); j++)
        {
            if (blockPosition[j] == position)
            {
                // Returning index to block we are looking for
                return i;
            }
        }
    }
    return -1;
}

void MainGame::MoveUp()
{
    // Moving all blocks one tile up
    for (int i = 0; i < m_blocks.size(); i++)
    {
        std::vector <glm::ivec2> blockPosition = m_blocks[i]->GetPosition();
        for (int j = 0; j < blockPosition.size(); j++)
        {
            blockPosition[j].y -= 1;
        }
        m_blocks[i]->SetPosition(blockPosition);
    }
}

void MainGame::InitNewBlocks()
{
    m_level.InitNewBlocks(m_newBlocks);

    m_newBlocksData = m_level.GetNewLinesData();

    // Rolling new blocks
    for (int y = 0; y < m_newBlocksData.size(); y++)
    {
        for (int x = 0; x < m_newBlocksData[y].size(); x++)
        {
            char tile = m_newBlocksData[y][x];
            if (tile == '.')
            {
                if (m_level.InsertBlock({ x, y }, m_newBlocksData, m_newBlocks, false, m_blockTypes))
                {
                    // Some stuff
                }
            }
        }
    }
}

void MainGame::AddNewBlocks(int newLines)
{
    for (int i = 0; i < m_newBlocksData[m_newLines].size(); i++)
    {
        char tile = m_newBlocksData[newLines][i];
        // If tile is a block
        if (tile != '#' && tile != '.')
        {
            // Getting index of block
            int index = FindBlock({ i, newLines }, m_newBlocks);
            if (index != -1)
            {
                Block* tmp_block = new Block;
                // Getting position of block
                std::vector <glm::ivec2> blockPosition = m_newBlocks[index]->GetPosition();
                // Adding his squares into leve data
                for (int i = 0; i < blockPosition.size(); i++)
                {
                    tmp_block->AddSquare(blockPosition[i].x, blockPosition[i].y + 16 - newLines);
                }
                // Pushing back to block types
                m_blockTypes.push_back(m_newBlocks[index]->GetShape());

                // Pushing back to blocks
                tmp_block->AddShape(m_newBlocks[index]->GetShape());
                m_blocks.push_back(tmp_block);

                // Deleting block from new level data
                delete m_newBlocks[index];
                m_newBlocks[index] = m_newBlocks.back();
                m_newBlocks.pop_back();
            }
        }
    }
}

void MainGame::InitQueue()
{
    if (!m_blockTypes.empty())
    {
        // Pushing new blocks to the queue
        for (int i = 0; i < QUEUE_SIZE; i++)
        {
            // Random generator
            static std::mt19937 randomEngine(time(NULL));
            std::uniform_int_distribution <int> roll(0, m_blockTypes.size() - 1);
            int index = roll(randomEngine);
            int value = 0;

            m_stackQueue.push_back(m_blockTypes.front());
            m_blockTypes.pop_front();
        }
    }
}

bool MainGame::UpdateQueue()
{
    // If we deleted all blocks we are quiting game
    if (m_blockTypes.empty())
    {
        m_stackQueue.push_back(m_emptyShape);
        return false;
    }
    else
    {
        while (m_stackQueue.size() < 3)
        {
            m_stackQueue.push_back(m_blockTypes.front());
            m_blockTypes.pop_front();
            return true;
        }
    }
}

bool MainGame::DrawQueue(std::vector <std::string>& data)
{
    int counter = 0;
    for (auto it = m_stackQueue.begin(); it != m_stackQueue.end(); it++)
    {
        std::vector <std::string> tmp_shape = it->GetShape();
        for (int i = 0; i < tmp_shape.size(); i++)
        {
            for (int j = 0; j < tmp_shape[i].size(); j++)
            {
                char tile = tmp_shape[i][j];
                if (tile != '.')
                {
                    data[QUEUE_POSITIONS[counter].y + i][QUEUE_POSITIONS[counter].x + j] = tile;
                }
            }
        }
        counter++;
    }
    return true;
}

void MainGame::RemoveQueue(std::vector <std::string>& data)
{
    int counter = 0;
    for (auto it = m_stackQueue.begin(); it != m_stackQueue.end(); it++)
    {
        std::vector <std::string> tmp_shape = it->GetShape();
        for (int i = 0; i < tmp_shape.size(); i++)
        {
            for (int j = 0; j < tmp_shape[i].size(); j++)
            {
                data[QUEUE_POSITIONS[counter].y + i][QUEUE_POSITIONS[counter].x + j] = '.';
            }
        }
        counter++;
    }
    m_stackQueue.pop_front();
}
