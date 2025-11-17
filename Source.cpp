#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace std;
using namespace sf;

class Maze {
public:
    int rows, cols;
    vector<vector<int>> grid;          // 0 = wall, 1 = path

    vector<pair<int, int>> mazeSteps;  // animation order

    Maze(int r, int c)
    {
        rows = r;
        cols = c;

        grid.assign(rows, vector<int>(cols, 0)); // all walls

        vector<vector<int>> visited(rows, vector<int>(cols, 0));

        DFS(1, 1, visited);
    }

    int dr[4] = { -2, 2, 0, 0 };
    int dc[4] = { 0, 0, -2, 2 };

    void DFS(int r, int c, vector<vector<int>>& visited)
    {
        visited[r][c] = 1;
        mazeSteps.push_back({ r, c }); // open this cell later (animation)

        // random direction order
        int order[4] = { 0, 1, 2, 3 };

        for (int i = 0; i < 4; i++)
        {
            int j = rand() % 4;
            swap(order[i], order[j]);
        }

        for (int i = 0; i < 4; i++)
        {
            int d = order[i];
            int nr = r + dr[d];
            int nc = c + dc[d];

            // borders
            if (nr <= 0 || nr >= rows - 1 || nc <= 0 || nc >= cols - 1)
                continue;

            if (!visited[nr][nc]) 
            {
                // open wall between two cells
                int wr = r + dr[d] / 2;
                int wc = c + dc[d] / 2;

                mazeSteps.push_back({ wr, wc }); // this wall becomes path

                DFS(nr, nc, visited);
            }
        }
    }
};

int main()
{
    srand(static_cast<unsigned int>(time(0)));

    int rows = 11;
    int cols = 21;
    const int CELL = 42;

    Maze maze(rows, cols);

    RenderWindow window(VideoMode(cols * CELL, rows * CELL), "Maze");

    // Game state
    bool mazeFinished = false;
    bool treasurePlaced = false;
    bool hasWon = false;

    int playerRow = 1, playerCol = 1;
    int treasureRow = -1, treasureCol = -1;

    // Maze animation
    Clock clock;
    int step = 0;


    Texture wallTex, groundTex, winTex;
    Texture treasureTex;
    Texture playerUpTex, playerLeftTex, playerRightTex, playerDownTex;

    wallTex.loadFromFile("Tile_15.png");
    groundTex.loadFromFile("ground2.png");
    winTex.loadFromFile("win.png");
    treasureTex.loadFromFile("pork-leg.png");

    playerUpTex.loadFromFile("walk up1.png");
    playerLeftTex.loadFromFile("walk left1.png");
    playerRightTex.loadFromFile("walk right1.png");
    playerDownTex.loadFromFile("walk down1.png");


   
    Sprite wall(wallTex);
    Sprite ground(groundTex);
    Sprite winSprite(winTex);
    Sprite playerSprite(playerRightTex);
    Sprite treasureSprite(treasureTex);


    wall.setScale((float)CELL / wallTex.getSize().x, (float)CELL / wallTex.getSize().y);
    ground.setScale((float)CELL / groundTex.getSize().x, (float)CELL / groundTex.getSize().y);

    // Win 
    FloatRect winBounds = winSprite.getLocalBounds();
    winSprite.setOrigin(winBounds.width / 2, winBounds.height / 2);
    winSprite.setPosition(window.getSize().x / 2, window.getSize().y / 2);

    // Player 
    FloatRect pBounds = playerSprite.getLocalBounds();
    playerSprite.setOrigin(pBounds.width / 2, pBounds.height / 2);
    playerSprite.setScale(3.9f, 3.9f);

    // Treasure 
    FloatRect tBounds = treasureSprite.getLocalBounds();
    treasureSprite.setOrigin(tBounds.width / 2, tBounds.height / 2);
    treasureSprite.setScale(0.33f, 0.33f);



    while (window.isOpen())
    {
        Event e;
        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                window.close();

            Texture* moveTex = &playerRightTex;

            if (e.type == Event::KeyPressed && mazeFinished && !hasWon)
            {
                int nr = playerRow, nc = playerCol;

                if (e.key.code == Keyboard::Up) { nr--; moveTex = &playerUpTex; }
                if (e.key.code == Keyboard::Down) { nr++; moveTex = &playerDownTex; }
                if (e.key.code == Keyboard::Left) { nc--; moveTex = &playerLeftTex; }
                if (e.key.code == Keyboard::Right) { nc++; moveTex = &playerRightTex; }

                if (nr >= 0 && nr < maze.rows &&
                    nc >= 0 && nc < maze.cols &&
                    maze.grid[nr][nc] == 1)
                {
                    playerRow = nr;
                    playerCol = nc;
                    playerSprite.setTexture(*moveTex);
                }

                if (playerRow == treasureRow && playerCol == treasureCol)
                    hasWon = true;
            }

            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape)
                window.close();
        }


        // animation
        if (!mazeFinished && step < maze.mazeSteps.size()
            && clock.getElapsedTime().asMilliseconds() > 35)
        {
            auto [r, c] = maze.mazeSteps[step];
            maze.grid[r][c] = 1;
            step++;
            clock.restart();

            if (step == maze.mazeSteps.size())
                mazeFinished = true;
        }


        if (mazeFinished && !treasurePlaced)
        {
            while (true)
            {
                int i = rand() % maze.mazeSteps.size();
                auto [tr, tc] = maze.mazeSteps[i];

                if (!(tr == playerRow && tc == playerCol)) {
                    treasureRow = tr;
                    treasureCol = tc;
                    treasurePlaced = true;
                    break;
                }
            }
        }


        window.clear();

        for (int r = 0; r < maze.rows; r++) {
            for (int c = 0; c < maze.cols; c++) {
                float x = c * CELL;
                float y = r * CELL;
                if (maze.grid[r][c] == 0) { wall.setPosition(x, y); window.draw(wall); }
                else { ground.setPosition(x, y); window.draw(ground); }
            }
        }

        if (treasurePlaced) {
            treasureSprite.setPosition(treasureCol * CELL + CELL / 2, treasureRow * CELL + CELL / 2);
            window.draw(treasureSprite);
        }

        if (mazeFinished) {
            playerSprite.setPosition(playerCol * CELL + CELL / 2, playerRow * CELL + CELL / 2);
            window.draw(playerSprite);
        }

        if (hasWon)
            window.draw(winSprite);

        window.display();
    }

    return 0;
}