#pragma once
#include <SDL/SDL.h>
#include <SDL/SDL_timer.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include <list>

#include "Window.h"
#include "InputManager.h"
#include "Level.h"
#include "Sprite.h"
#include "Block.h"
#include "Shape.h"
#include "FPSLimiter.h"

enum class GameState{PLAY, EXIT, TUTORIAL};

class MainGame
{
public:
    MainGame();
    ~MainGame();

    void Run();

private:
    // Functions
    void InitSystems();
    void GameLoop();
    void Draw(std::vector <std::string>& data);
    void Update();
    void UpdateBlocks();
    void ProcessInput();
    void InitLevel();
    void PrintEndscreen();

    void InitTutorial();
    void PlayTutorial(SDL_Rect& destRect, int part);
    bool UpdateTutorial(SDL_Rect& destRect);

    bool InitBlocks(glm::vec2 position);
    void RemoveBlock(std::vector <std::string>& data);
    void ProcessRemove(int index, std::vector <std::string>& data);

    int FindBlock(glm::ivec2 position, std::vector <Block*>& blocks);

    void MoveUp();


    // New blocks func
    void InitNewBlocks();
    void AddNewBlocks(int lanes);


    // Queue functions
    void InitQueue();
    bool UpdateQueue();
    bool DrawQueue(std::vector <std::string>& data);
    void RemoveQueue(std::vector <std::string>& data);



    // Classes
    Window m_window;
    InputManager m_inputManager;
    Level m_level;
    Sprite m_sprite;
    FPSLimiter m_fps;

    // Game renderer
    SDL_Renderer* m_renderer;

    // Textures
    SDL_Texture* m_levelTexture;

    // Blocks textures
    SDL_Texture* m_blueSquare;
    SDL_Texture* m_redSquare;
    SDL_Texture* m_greenSquare;
    SDL_Texture* m_purpleSquare;
    SDL_Texture* m_yellowSquare;
    SDL_Texture* m_pinkSquare;
    SDL_Texture* m_graySquare;
    SDL_Texture* m_border;
    SDL_Texture* m_fingerTexture;

    // Font textures
    SDL_Texture* m_textTexture;
    SDL_Texture* m_tapHereTexture;
    TTF_Font* m_font;

    // Shapes
    Shape LShape;
    Shape RLShape;
    Shape ZShape;
    Shape RZShape;
    Shape SQShape;
    Shape ZShape2;
    Shape RZShape2;

    Shape m_emptyShape;

    Mix_Music* m_music;
    Mix_Chunk* m_destroy;
    Mix_Chunk* m_fast;
    Mix_Chunk* m_impressive;

    // Hold all blocks
    std::vector <std::string> m_levelData;
    std::vector <Block*> m_blocks;
    //std::vector <Block*> m_tutBlocks;

    // Hold all new blocks
    std::vector <std::string> m_newBlocksData;
    std::vector <Block*> m_newBlocks;

    // Tutorial data
    std::vector <std::string> m_tutorialData;
    std::vector <std::string> m_tutorialData2;

    // All shapes that exsist on map for making valid queue
    std::list <Shape> m_blockTypes;

    // List of blocks in queue
    std::list <Shape> m_stackQueue;

    // Queue position
    static const int QUEUE_SIZE = 3;
    const glm::vec2 QUEUE_POSITIONS[QUEUE_SIZE] = { {18, 2}, {18, 6}, {18, 10} };

    // Hold game state
    GameState m_gameState;

    // Textures width
    const int TILE_WIDTH = 32;

    // Speed of blocks moving up
    float m_speed;

    float m_multiplier;

    // Number of new lanes already appeared
    int m_newLines;

    // Pplayer score
    int m_score;

    // Hold time while tutorial is on
    float m_tutorialTime;


    bool m_tutorialUpdate;
};

